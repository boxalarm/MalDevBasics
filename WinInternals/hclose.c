// Force closes a handle in a target process using DuplicateHandle w/ DUPLICATE_CLOSE_SOURCE

// To test this, open wmplayer.exe and use Process Explorer to find its PID
// In the handle table search for the mutex object `Microsoft_WMP_70_CheckForOtherInstanceMutex` and grab its handle value
// Supply the PID and handle value as args to hclose.exe

// Once you run this program, the CheckForOtherInstanceMutex object will be closed inside the 
// wmplayer.exe process which will allow you to open up another instance of wmplayer.exe (which is prevented when that object exists)

#include <Windows.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
	if (argc != 3) {
		printf("Usage: hclose.exe [pid] [handle]\n");
		printf("Example: hclose.exe 1520 0xD8\n");

		return 1;
	}

	// OpenProcess expects the PID to be a DWORD (unsigned long)
	// The PID is the target process whose handle we are duplicating
	DWORD pid = strtoul(argv[1], NULL, 10);

	// hTarget is the handle within `pid` that we are duplicating
	HANDLE hTarget = (HANDLE)strtoul(argv[2], NULL, 16);

	printf("[+] Target PID   : %lu\n", pid);
	printf("[+] Target handle: 0x%p\n", hTarget);

	// Open a handle to the target process 
	HANDLE hProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, pid);
	if (!hProcess) {
		printf("[-] OpenProcess failed: %u\n", GetLastError());
		return 1;
	}

	// This is where we'll store the duplicated handle
	HANDLE hDup = NULL;

	printf("[+] Opened a handle to our target process...\n");

	// Now we'll duplicate the handle 
	printf("[+] Duplicating handle with DUPLICATE_CLOSE_SOURCE...\n");
	if (DuplicateHandle(hProcess,
		hTarget,
		NULL, // Can be NULL because DUPLICATE_CLOSE_SOURCE is being used
		&hDup,
		SYNCHRONIZE,
		FALSE,
		DUPLICATE_CLOSE_SOURCE // The source handle in our target process (e.g., wmplayer.exe) will automatically be closed
	)) {
		printf("[+] Handle 0x%p successfully closed in target process!\n", hTarget);
		CloseHandle(hDup);
	}
	else {
		printf("[-] DuplicateHandle failed: %u", GetLastError());
	}
	CloseHandle(hProcess);
	return 0;
}
