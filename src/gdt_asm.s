; This will set up our new segment registers. We need to do
; something special in order to set CS. We do what is called a
; far jump. A jump that includes a segment as well as an offset.
; This is declared in C as 'extern void gdt_flush();'

global tss_flush
extern TSS_SEGMENT

tss_flush:
  mov ax, 0x38
  ltr ax
  ret

global gdt_flush                ; Allows the C code to link to this
extern gp                       ; Says that 'gp' is in another file
extern KERNEL_DATA_SEGMENT
extern KERNEL_STACK_SEGMENT
extern KERNEL_CODE_SEGMENT

gdt_flush:
  lgdt [gp]                     ; Load the GDT with our '_gp' which is a special pointer
  mov ax, 0x10
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ax, 0x18
  mov ss, ax
  jmp 0x8:flush2 ; Far jump!

flush2:
  ret                           ; Returns back to the C code!
