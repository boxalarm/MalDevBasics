; Example calling the WinAPI to create a file

externdef CreateFileA:proc
externdef WriteFile:proc
externdef CloseHandle:proc
externdef GetLastError:proc

OPEN_ALWAYS = 4
GENERIC_WRITE = 40000000h

.code
main proc
	; function prologue 
	push rbp               ; save rbp on the stack for later - this also gets us to 16 byte stack alignment
	mov rbp, rsp           ; save rsp in rbp
	sub rsp, 8             ; make room for local vars

	written equ [rbp - 4]  ; defines 'written' as an offset from rbp (the top 4 bytes of the 16 bytes we set aside for local vars)
	
	; set up for call to CreateFileA
	lea rcx, filename      ; lpFileName
	mov edx, GENERIC_WRITE ; dwDesiredAccess
	xor r8d, r8d           ; dwShareMode (null)
	xor r9, r9             ; lpSecurityAttributes (null)
	push 0                 ; 7th param: hTemplateFile
	push 0                 ; 6th param: dwFlagsAndAttributes
	push OPEN_ALWAYS       ; 5th param: dwCreationDisposition - after this instruction the stack is 8 byte aligned
	sub rsp, 20h           ; allocate spill space
	call CreateFileA
	test eax, eax          ; if the call fails it will return a negative value and the sign flag will be set
	js error               ; if sign flag is set (call failed), jump to error
	
	; set up for call to WriteFile
	mov rcx, rax           ; hFile - CreateFileA returned the handle and placed it in rax
	lea rdx, text          ; lpBuffer
	mov r8, sizeof text    ; nNumberOfBytesToWrite
	lea r9, written        ; lpNumberOfBytesWritten - written is equal to [rbp - 4]
	mov rbx, rcx           ; save rcx (file handle) for CloseHandle
	push 0                 ; lpOverlapped (null)
	and rsp, -16           ; align stack
	call WriteFile
	test eax, eax          ; non-zero value indicates success
	jz error               ; if zero, jump to error
	
	; set up for call to CloseHandle
	mov rcx, rbx           ; hObject - file handle
	call CloseHandle
	test eax, eax          ; non-zero value indicates success
	jz error               ; if zero, jump to error
	ret
	
	error:
		call GetLastError
		ret
main endp

.data
filename db "c:\code\hello2.txt",0
text     db "hello world"

end
