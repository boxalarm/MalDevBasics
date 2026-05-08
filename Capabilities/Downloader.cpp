// Simple COM client PoC to demonstrate how to download a file using BITS (Background Intelligent Transfer Service)
// Helpful docs: https://learn.microsoft.com/en-us/windows/win32/bits/using-bits
// 
// To test, modify line 32 with the remote URL and local file path of your choosing
// 
// Compile with: cl downloader.cpp /link bits.lib ole32.lib

#include <Windows.h>
#include <stdio.h>
#include <Bits.h>

int main() {
	// Initialize COM
	CoInitialize(NULL);

	// The BackgroundCopyManager class implements the IBackgroundCopyManager interface
	IBackgroundCopyManager* pMgr;
	HRESULT hr = CoCreateInstance(CLSID_BackgroundCopyManager, NULL,
		CLSCTX_ALL, IID_IBackgroundCopyManager, (void**)&pMgr);

	if (SUCCEEDED(hr)) {
		// Create a new job (download)
		// OPSEC NOTE: "Download job" shows up when you run `bitsadmin /list`
		GUID jobId; // Unique ID for the job that we don't care about
		IBackgroundCopyJob* pJob; 
		hr = pMgr->CreateJob(L"Download job", BG_JOB_TYPE_DOWNLOAD, &jobId, &pJob);
		
		// We can release pMgr because the job keeps it alive
		pMgr->Release();
		
		// Provide remote URL and local file path to AddFile
		hr = pJob->AddFile(L"http://x.x.x.x/test.txt", L"C:\\path\\to\\test.txt");
		hr = pJob->Resume(); // Resume is what starts the job
		
		if (SUCCEEDED(hr)) {
			printf("Downloading...");

			// Poll job state 
			// Infinite loop that breaks if download completes or an error occurs
			BG_JOB_STATE state;
			for (;;) {
				pJob->GetState(&state);
				if (state == BG_JOB_STATE_ERROR || state == BG_JOB_STATE_TRANSFERRED)
					break;
				printf(".");
				Sleep(300);
			}
			// We need to check if the transfer was completed or if we got an error
			if (state == BG_JOB_STATE_TRANSFERRED) {
				pJob->Complete(); // The file is only saved after Complete() is called
				printf("Done!\n");
			}
			else {
				printf("Error!\n");
			}
			pJob->Release();
		}
	}
	// Cleanup
	CoUninitialize();
}
