// Learning how to use various WinAPI functions to display system information

#include <Windows.h>
#include <stdio.h>
#include <Psapi.h>

int main()
{
	// GetSystemInfo
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	printf("---- GetSystemInfo ----\n\n");
	printf("Minimum application address: 0x%p\n", si.lpMinimumApplicationAddress);
	printf("Maximum application address: 0x%p\n", si.lpMaximumApplicationAddress);
	printf("Number of processors: %u\n", si.dwNumberOfProcessors);
	printf("Processor type: %u\n", si.dwProcessorType);

	// GetComputerNameW
	// TCHAR is a macro that maps to wchar_t because we're using GetComputerNameW
	TCHAR compName[MAX_COMPUTERNAME_LENGTH + 1] = { 0 };
	DWORD size = _countof(compName); 
	
	if (GetComputerNameW(compName, &size)) {
		printf("\n---- GetComputerName ----\n\n");
		printf("Computer name: %ws\n", compName);
	}
	else {
		printf("Error retrieving computer name (%u)\n", GetLastError());
	}

	// GetComputerNameExW
	TCHAR dnsName[256] = { 0 };
	DWORD size_dns = _countof(dnsName);

	if (GetComputerNameExW(ComputerNameDnsHostname, dnsName, &size_dns)) {
		printf("\n---- GetComputerNameEx ----\n\n");
		printf("DNS Hostname: %ws\n", dnsName);
	}
	else {
		printf("Error retrieving DNS name (%u)\n", GetLastError());
	}

	// GetSystemDirectory
	TCHAR winDir[MAX_PATH] = { 0 };
	
	// We can pass _countof because the parameter is a UINT and not a pointer (like LPDWORD)
	if (GetSystemDirectoryW(winDir, _countof(winDir))) {
		printf("\n---- GetSystemDirectoryW ----\n\n");
		printf("System dir: %ws\n", winDir);
	}
	else {
		printf("Error retrieving directory (%u)\n", GetLastError());
	}

	// GetUserName
	TCHAR userName[256] = { 0 };
	DWORD size_user = _countof(userName);

	if (GetUserNameW(userName, &size_user)) {
		printf("\n---- GetUserNameW ----\n\n");
		printf("Username: %ws\n", userName);
	}
	else {
		printf("Error retrieving username (%u)\n", GetLastError());
	}
	
	// GetPerformanceInfo
	PERFORMANCE_INFORMATION pi;

	if (GetPerformanceInfo(&pi, sizeof(pi))) {
		printf("\n---- GetPerformanceInfo ----\n\n");
		printf("Handle count: %u\n", pi.HandleCount);
		printf("Thread count: %u\n", pi.ThreadCount);
		printf("Process count: %u\n", pi.ProcessCount);
	}
	else {
		printf("Error retrieving info (%u)\n", GetLastError());
	}

	return 0;
}
