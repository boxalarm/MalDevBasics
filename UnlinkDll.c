// Inject this DLL into a target process and it will hide itself by bypassing its links in the PEB
// NOTE: Process Explorer will still see it but using listdlls.exe (and other tools) won't
// Relied heavily on code from: https://blog.christophetd.fr/dll-unlinking/

// For this to work you need to compile this DLL with the name "UnlinkDll.dll" (or modify the code - line 42)
// Compile with: cl /LD UnlinkDll.c

#include <Windows.h>
#include <winternl.h>
#include <stdio.h>

// We have to redefine the LDR_DATA_TABLE_ENTRY struct because winternl.h only exposes
// InMemoryOrderLinks but not the other two LIST_ENTRY lists that we need 
// (InLoadOrderLinks and InInitializationOrderLinks)
typedef struct _MY_LDR_DATA_TABLE_ENTRY
{
    LIST_ENTRY      InLoadOrderLinks;
    LIST_ENTRY      InMemoryOrderLinks;
    LIST_ENTRY      InInitializationOrderLinks;
    PVOID           DllBase;
    PVOID           EntryPoint;
    ULONG           SizeOfImage;
    UNICODE_STRING  FullDllName;
    UNICODE_STRING  ignored;
    ULONG           Flags;
    SHORT           LoadCount;
    SHORT           TlsIndex;
    LIST_ENTRY      HashTableEntry;
    ULONG           TimeDateStamp;
} MY_LDR_DATA_TABLE_ENTRY;

void unlink_dll(void) {
    PEB* peb = NtCurrentTeb()->ProcessEnvironmentBlock; // this gives us the pointer to the PEB

    LIST_ENTRY* head = &peb->Ldr->InMemoryOrderModuleList;

    LIST_ENTRY* next = head->Flink;
    while (next != head) {
        MY_LDR_DATA_TABLE_ENTRY* entry = CONTAINING_RECORD(next, MY_LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
        char dllName[256];
        snprintf(dllName, sizeof(dllName), "%wZ", entry->FullDllName);
        if (strstr(dllName, "UnlinkDll.dll") != NULL) {
            // We found the DLL - now we need to unlink it from all 3 doubly-linked lists
            entry->InLoadOrderLinks.Blink->Flink = entry->InLoadOrderLinks.Flink;
            entry->InLoadOrderLinks.Flink->Blink = entry->InLoadOrderLinks.Blink;

            entry->InMemoryOrderLinks.Blink->Flink = entry->InMemoryOrderLinks.Flink;
            entry->InMemoryOrderLinks.Flink->Blink = entry->InMemoryOrderLinks.Blink;

            entry->InInitializationOrderLinks.Blink->Flink = entry->InInitializationOrderLinks.Flink;
            entry->InInitializationOrderLinks.Flink->Blink = entry->InInitializationOrderLinks.Blink;

            return;
        }
      
        next = next->Flink;
    }
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
        unlink_dll();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
