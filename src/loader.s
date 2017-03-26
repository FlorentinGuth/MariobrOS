global loader                     ; The entry symbol for ELF (Executable and Linkable Format)

MAGIC_NUMBER equ 0x1BADB002       ; The magic number constant
FLAGS        equ 0x0              ; Multiboot flags
CHECKSUM     equ -MAGIC_NUMBER    ; Magic number + checksum + flags = 0


section .text                     ; Code section
align 4                           ; 4 byte-aligned code
  dd MAGIC_NUMBER                 ; Write the magic number to the machine code,
  dd FLAGS                        ; The flags,
  dd CHECKSUM                     ; And the checksum

loader:                           ; The loader label (defined as entr point in the linker script)
  mov eax, 0xDEADBEEF             ; Place whatever we want in the register eax
.loop:
  jmp .loop                       ; Loop forever
