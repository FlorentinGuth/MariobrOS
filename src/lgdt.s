global load_gdt		; Make the function visible outside the file
global set_segments	
	
load_gdt:
	mov eax, [esp+4]
	lgdt [eax]
	ret
	
set_segments:
	mov eax, 0x10
	mov ds, eax
	mov ss, eax
	mov es, eax

	jmp 0x08:flush_cs	; code here uses the previous cs
				; specify cs when jumping to flush_cs

flush_cs:	; now we’ve changed cs to 0x08
