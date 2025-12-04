// Enumerate all top-level windows and display their titles

#include <Windows.h>
#include <stdio.h>

// This is our callback function to pass to EnumWindows
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
	// Buffer that will receive the window title
	TCHAR title[256] = { 0 };
	
	// GetWindowText returns 0 if the title is empty or the handle is invalid
	if (GetWindowTextW(hwnd, title, _countof(title)) > 0) {
		printf("Window: %ws (Handle: 0x%p)\n", title, hwnd);
	}
	else {
		printf("Window: <no title>\n");
	}
	// We return TRUE so EnumWindows will continue until the last top-level window is enumerated
	return TRUE;
}

int main() {
	printf("[+] Enumerating all windows...\n\n");
	// NOTE: EnumWindows expects a function pointer — function names automatically decay to pointers
	EnumWindows(EnumWindowsProc, 0);
	printf("\n[+] Done!");
}
