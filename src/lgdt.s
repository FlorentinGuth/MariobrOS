global load_gdt			; Make the function visible outside the file
	
load_gdt:
	mov eax, [esp+4]
	lgdt [eax]
	ret
	
