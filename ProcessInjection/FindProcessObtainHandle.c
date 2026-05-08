// Compile with VS build tools:
// cl .\FindProcessObtainHandle.c /DUNICODE /D_UNICODE

#include <Windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// Finds a process by name (case-insensitive) and returns its PID and an opened handle
// Returns true on success (PID found and handle opened), false otherwise
bool FindProcessObtainHandle(const wchar_t* processName, DWORD* pid, HANDLE* hProcess) {
	if (processName == NULL || pid == NULL) {
		return false;
	}

	*pid = 0;
	*hProcess = NULL;

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE) {
		printf("[-] CreateToolhelp32Snapshot failed: %d\n", GetLastError);
		return false;
	}

	// Need to initialize dwSize to the size in bytes of the struct
	PROCESSENTRY32W pe32; 
	pe32.dwSize = sizeof(pe32);

	// Grab information about the first process in the snapshot
	if (!Process32FirstW(hSnapshot, &pe32)) {
		printf("[-] Process32FirstW failed: %d\n", GetLastError());
		return false;
	}

	do {
		// Perform case insensitive search - 0 = match
		if (_wcsicmp(pe32.szExeFile, processName) == 0) {
			*pid = pe32.th32ProcessID;

			// Open a handle to the process
			*hProcess = OpenProcess(
				PROCESS_CREATE_THREAD | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, *pid);
			if (!*hProcess) {
				printf("[-] OpenProcess failed: %d\n", GetLastError());
				return false;
			}

			CloseHandle(hSnapshot);
			return true;
		}
	} while (Process32NextW(hSnapshot, &pe32));

	CloseHandle(hSnapshot);
	return false;
}

// Using wmain because we need to accept wide (Unicode) strings as an argument 
int wmain(int argc, wchar_t* argv[]) {
	if (argc < 2) {
		printf("Usage: FindProcessObtainHandle.exe [process_name]\n");
		return 1;
	}

	DWORD pid = 0;
	HANDLE hProcess = NULL;

	// Find the process and obtain a handle to that process
	if (!FindProcessObtainHandle(argv[1], &pid, &hProcess)) {
		wprintf(L"[-] Process '%ws' not found\n", argv[1]);
	}
	else {
		wprintf(L"[+] PID for %ws: %lu\n", argv[1], pid);
		wprintf(L"[+] Handle obtained: 0x%x\n", hProcess);
	}

	return 0;
}
