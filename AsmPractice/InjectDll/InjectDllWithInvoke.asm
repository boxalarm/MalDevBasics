.code

; use the MASM64 SDK
include \masm64\include64\masm64rt.inc

Inject proc
; params: uint32_t pid (ecx because it's a 4 byte value), char* dll_path (rdx)
	mov r12, rdx				; save dllPath to r12 (because rdx will be overwritten when setting up params for OpenProcess call)
	mov eax, ecx				; move PID to eax because the invoke macro will place our first arg to OpenProcess in RCX which will override ECX
	
	; get a handle to target process
	invoke OpenProcess, PROCESS_CREATE_THREAD or PROCESS_VM_OPERATION or PROCESS_VM_WRITE, NULL, eax
	test rax, rax				; OpenProcess returns NULL if it fails
	jz exit						  ; if ZF set, call failed (NULL was returned)
	mov r13, rax				; save our process handle that was returned

	; allocate memory in target process
	invoke VirtualAllocEx, r13, NULL, 4096, MEM_COMMIT or MEM_RESERVE, PAGE_READWRITE
	test rax, rax
	jz exit
	mov r14, rax				; save base address of allocated memory

	; write dll path
	invoke WriteProcessMemory, r13, r14, r12, rv(AsmStrLen, r12), NULL
	test eax, eax				; WriteProcessMemory returns a BOOL, which is a typedef for int in the WinAPI (so its 4 bytes)
	jz exit

	; get the address for LoadLibrary
	invoke GetProcAddress, rv(GetModuleHandleA, "kernel32"), "LoadLibraryA"
	test rax, rax
	jz exit

	; create the remote thread and execute our dll
	invoke CreateRemoteThread, r13, NULL, 0, rax, r14, 0, NULL
	test eax, eax
	jz exit

	; close handles
	invoke CloseHandle, rax		  ; remote thread handle
	invoke CloseHandle, r13     ; process handle

	mov rax, 1					; return TRUE indicating success
	ret
exit:
	; the value of rax will be 0 before jumping here, which returns FALSE
	ret

Inject endp

AsmStrLen proc
; takes a string as a parameter (rcx)
xor rax, rax			    ; zero out our counter (this is our return value also)

charcount:
	cmp byte ptr [rcx], 0   ; check if first byte is null terminated (indicating end of string)
	jz exit			        ; jump to end_prog if null terminator exists

	inc rax				    ; increment our counter
	inc rcx				    ; move to the next byte of our string
	jmp charcount           ; go back to beginning of loop

exit:
	ret
AsmStrLen endp

.data

end
