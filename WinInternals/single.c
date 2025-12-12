// These were the lab instructions from TrainSec:
// 
// Use a mutex to ensure there is just a single instance of single.exe running at a time.
// Follow these guidelines :
//  a.Create a named mutex.
//  b.If the call succeeds but GetLastError returns ERROR_ALREADY_EXISTS, it
//    means the mutex has already existed before the call.
//  c.If this is the case, display a message to that effect and quit.
//  d.Otherwise, display a different message and keep the process alive.

#include <Windows.h>
#include <stdio.h>

int main() {
	// Create a named mutex
	HANDLE hMutex = CreateMutexW(NULL, TRUE, L"MyNamedMutex");

	// Check if the named mutex already exists
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		printf("Mutex already exists\n");

		// Exit the process - could also call exit(0)
		return 0;
	}
	else {
		printf("Mutex has just been created - press ENTER to exit\n");
		getchar(); // holds the process open until ENTER is pressed
		
		return 0;
	}
}
