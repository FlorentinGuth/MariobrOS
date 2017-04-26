
	.set ALIGN,     1<<0
	.set MEMINFO,   1<<1
	.set FLAGS,     ALIGN | MEMINFO
	.set MAGIC,     0x1BADB002
	.set CHECKSUM,  -(MAGIC + FLAGS)


	.section .multiboot
	.align 4
	.long MAGIC
	.long FLAGS        
	.long CHECKSUM

	.code32

	.section .bootstrap_stack
stack_bootom:
	.skip 16384 # 16 KiB

stack_top:
	.section .text

        .global loader
loader: 
        movl $stack_top, %esp
        push %ebx
        call kmain

.loop:
        hlt
        jmp .loop                     ; Loop forever

