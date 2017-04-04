global loader                   ; The entry symbol for ELF (Executable and Linkable Format)
extern kmain                    ; The kmain function is not defined her (in kmain.c)

MAGIC_NUMBER equ 0x1BADB002     ; The magic number constant
FLAGS        equ 0x0            ; Multiboot flags
CHECKSUM     equ -MAGIC_NUMBER  ; Magic number + checksum + flags = 0

KERNEL_STACK_SIZE equ 4096      ; Define a stack of 4 KB

section .bss                    ; Uninitialized data section
align 4                         ; Align at 4 bytes
kernel_stack:                   ; Label points to beginning of memory
  resb KERNEL_STACK_SIZE        ; Reserve stack for the kernel

section .text                   ; Code section
align 4                         ; 4 byte-aligned code
  dd MAGIC_NUMBER               ; Write the magic number to the machine code,
  dd FLAGS                      ; The flags,
  dd CHECKSUM                   ; And the checksum

loader:                         ; The loader label (defined as entry point in the linker script)
  mov eax, 0xDEADBEEF           ; Place whatever we want in the register eax
  mov esp, kernel_stack + KERNEL_STACK_SIZE ; Points esp to the start of the stack (end of memory area)

  ; Push arguments to the stack, from last to first
  call kmain                    ; Call the kmain function from kmain.c (return in eax)
  cli                           ; Prevents further interruptions

.loop:
  hlt                     ; Loop forever
