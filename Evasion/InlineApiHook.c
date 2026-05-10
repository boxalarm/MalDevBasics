// PoC: Inline API hooking with a trampoline targeting NtAllocateVirtualMemory
// Demonstrates the hook mechanism used by some EDRs in user mode
// 
// Build: Visual Studio DLL project, disable precompiled headers
// Load into a target process with LoadLibraryA("InlineApiHook.dll") (or inject it remotely)

#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <winternl.h>
#include <stdio.h>

#define HOOK_SIZE 13        // x64 only
#define ORIG_BYTES_SIZE 16  // covers 3 complete instructions in NtAllocateVirtualMemory:
                            // mov r10, rcx (3) + mov eax, 18h (5) + test byte ptr [...] (8)
                            // must cover complete instructions to avoid trampoline corruption

typedef NTSTATUS(NTAPI* pNtAllocateVirtualMemory)(
    HANDLE    ProcessHandle,
    PVOID* BaseAddress,
    ULONG_PTR ZeroBits,
    PSIZE_T   RegionSize,
    ULONG     AllocationType,
    ULONG     Protect
    );

// Global vars
BYTE  g_OriginalBytes[ORIG_BYTES_SIZE] = { 0 };
PVOID g_pNtAllocate = NULL;
PVOID g_pTrampoline = NULL;

// This is our detour function
NTSTATUS NTAPI HookedNtAllocateVirtualMemory(
    HANDLE    ProcessHandle,
    PVOID* BaseAddress,
    ULONG_PTR ZeroBits,
    PSIZE_T   RegionSize,
    ULONG     AllocationType,
    ULONG     Protect)
{
    printf("[HOOK] NtAllocateVirtualMemory intercepted!\n");
    printf("       Size:    %zu bytes\n", *RegionSize);
    printf("       Protect: 0x%X\n", Protect);

    // Call the real NtAllocateVirtualMemory through the trampoline - in this case we're just passing all the parameters through untouched
    NTSTATUS result = ((pNtAllocateVirtualMemory)g_pTrampoline)(ProcessHandle, BaseAddress, ZeroBits, RegionSize, AllocationType, Protect);

    return result;
}

VOID InstallHook() {
    // This is the hook that we'll patch at the beginning of NtAllocateVirtualMemory to redirect
    // execution to our detour function 'HookedNtAllocateVirtualMemory'
    BYTE hook[HOOK_SIZE] = {
        0x49, 0xBB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov r11, <address>
        0x41, 0xFF, 0xE3                                            // jmp r11
    };

    // Grab the address of the API we want to hook (NtAllocateVirtualMemory)
    g_pNtAllocate = GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtAllocateVirtualMemory");
    if (!g_pNtAllocate) {
        printf("[-] Couldn't get address: %d", GetLastError());
        return;
    }

    // Save the original bytes before we overwrite anything (this ends up being the first three instructions in NtAllocateVirtualMemory)
    memcpy(g_OriginalBytes, g_pNtAllocate, ORIG_BYTES_SIZE);

    // Allocate executable buffer for our trampoline
    g_pTrampoline = VirtualAlloc(NULL, ORIG_BYTES_SIZE + HOOK_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE); 
    if (!g_pTrampoline) {
        printf("[-] VirtualAlloc failed: %d\n", GetLastError());
        return;
    }

    // Copy the saved original bytes into the trampoline buffer
    memcpy(g_pTrampoline, g_OriginalBytes, ORIG_BYTES_SIZE);  // Right now our trampoline has the first 3 instructions of NtAllocateVirtualMemory

    // Patch the return address into our hook and append the hook to the trampoline
    // pReturnAddr = NtAllocateVirtualMemory + ORIG_BYTES_SIZE (past the bytes we saved)
    PVOID pReturnAddr = (PVOID)((PBYTE)g_pNtAllocate + ORIG_BYTES_SIZE);
    memcpy(&hook[2], &pReturnAddr, sizeof(PVOID));
    memcpy((PBYTE)g_pTrampoline + ORIG_BYTES_SIZE, hook, HOOK_SIZE); // This copies our hook to the end of the trampoline

    // Repatch with our detour functions address
    PVOID pHook = (PVOID)HookedNtAllocateVirtualMemory;
    memcpy(&hook[2], &pHook, sizeof(PVOID)); // Now hook holds the address of our detour function

    // Make NtAllocateVirtualMemory writable so we can write the patch (which redirects execution to our detour function), and finally restore the old protection
    DWORD dwOldProt = 0;
    VirtualProtect(g_pNtAllocate, HOOK_SIZE, PAGE_EXECUTE_READWRITE, &dwOldProt);
    memcpy(g_pNtAllocate, hook, HOOK_SIZE);
    VirtualProtect(g_pNtAllocate, HOOK_SIZE, dwOldProt, &dwOldProt);

    printf("[+] Hook installed on NtAllocateVirtualMemory\n");
    printf("[+] Trampoline at : 0x%p\n", g_pTrampoline);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        InstallHook();
    }
    return TRUE;
}
