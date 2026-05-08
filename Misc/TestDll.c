// DLL that can be used for testing - it'll pop a message box when successfully
// injected into a process

// INSTRUCTIONS: Compile using VS build tools
// cl /LD dlltest.c /DUNICODE /D_UNICODE /link user32.lib /OUT:dlltest.dll

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h> // required for _countof

void message(void) {
    // First get the current PID
    DWORD pid = GetCurrentProcessId();
    WCHAR buffer[64] = { 0 };
    // Convert DWORD to wide string which MessageBoxW requires
    swprintf(buffer, _countof(buffer), L"%lu", pid);

    // Call MessageBox to display the current process ID
    MessageBox(NULL, buffer, L"Procces ID", MB_OK | MB_ICONWARNING);
    return;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        message();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
