; implement various string CRT functions

.code

AsmStrLen proc
; takes a string as a parameter (rcx)
xor rax, rax			    ; zero out our counter (this is our return value also)

char_count:
	cmp byte ptr [rcx], 0   ; check if first byte is null terminated (indicating end of string)
	jz exit			        ; jump to end_prog if null terminator exists

	inc rax				    ; increment our counter
	inc rcx				    ; move to the next byte of our string
	jmp char_count          ; go back to beginning of loop

exit:
	ret
AsmStrLen endp

AsmStrCpy proc
; params (dest, source) = (rcx, rdx)

; strcpy returns a pointer to the dest (rcx) so we'll save that now before we modify it
mov r12, rcx

copyloop:
	mov al, [rdx]                ; copy source byte into byte register
	mov [rcx], al                ; copy into dest
	test al, al					         ; if ZF set, we've reached null terminator, exit
	jz exit
	inc rdx                      ; move to next source byte
	inc rcx                      ; move to next dest byte
	jmp copyloop

exit:
	mov rax, r12
	ret
AsmStrCpy endp

AsmStrCat proc
; params (dest, source) = (rcx, rdx)
; strcat appends a source string to the end of the destination string

; strcat returns a pointer to the dest (rcx) so we'll save that now before we modify it
mov r12, rcx

; scan dest byte by byte until 0 (null terminator) is reached
; this increments rcx (dest) after its null terminator which is where we 
; need to begin appending the src bytes to 
scanzero:
	cmp byte ptr [rcx], 0
	jz copyloop				           ; if we've reached null terminator we can start appending the source
	inc rcx						           ; this increments rcx by one byte
	jmp scanzero

; append source (rdx) to dest (rcx)
copyloop:
	mov al, [rdx]                ; copy source byte into byte register
	mov [rcx], al                ; copy into dest
	test al, al					         ; if ZF set, we've reached null terminator, exit
	jz exit
	inc rdx                      ; move to next source byte
	inc rcx                      ; move to next dest byte
	jmp copyloop

exit:
	mov rax, r12				; return pointer to destination string (saved into r12 earlier)
	ret

AsmStrCat endp

end
