; Assembler: MASM64
; Count all 1 bits in an unsigned 64 bit value

.code

BitCount proc
xor rax, rax        ; zero out counter

loop_start:
	shr rcx, 1        ; if a 1 is dropped with our shift it will set the carry flag
	jnc skip          ; if carry flag isn't set (because a 1 wasn't dropped) jump to skip (don't increment counter)
	inc rax           ; otherwise increment the counter
skip:
	test rcx, rcx     ; sets zero flag if RCX = 0
	jnz loop_start    ; if RCX is not zero, jump back to loop
	ret
BitCount endp

end
