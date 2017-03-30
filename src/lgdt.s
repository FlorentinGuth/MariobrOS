global load_gdt		; Make the function visible outside the file
global set_segments        

	
load_gdt:
	mov eax, [esp+4]
	lgdt [eax]
	ret
	
set_segments:
	mov ax, 0x10
	mov ds, ax
	mov ss, ax
	mov es, ax

	jmp 0x08:flush_cs	; specify cs when jumping to flush_cs

flush_cs:       
