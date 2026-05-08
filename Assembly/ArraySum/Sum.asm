; Assembler: MASM64

.code

Sum proc
	xor rax, rax      ; zero out rax - this holds our return value
loop_start:
	add rax, [rcx]    ; rcx holds first argument which is the base address of an int array (pointer to first element)
	add rcx, 4        ; an int is 4 bytes - this moves us to the next value in the array
	dec rdx           ; rdx is our second argument which holds the number of ints in the array
	jnz loop_start    ; when rdx gets to 0, the zero flag will be set and the loop will break
	ret
Sum endp

end
