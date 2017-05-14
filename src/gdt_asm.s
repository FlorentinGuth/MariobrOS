; This will set up our new segment registers. We need to do
; something special in order to set CS. We do what is called a
; far jump. A jump that includes a segment as well as an offset.
; This is declared in C as 'extern void gdt_flush();'

global tss_flush
tss_flush:
  mov ax, 0x3B
  ltr ax
  ret

global gdt_flush                ; Allows the C code to link to this
extern gp                       ; Says that 'gp' is in another file


gdt_flush:
  lgdt [gp]                     ; Load the GDT with our '_gp' which is a special pointer
  mov ax, 0x10                  ; 0x10 is the offset in the GDT to our data segment
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax
  jmp 0x08:flush2               ; 0x08 is the offset to our code segment: Far jump!

flush2:
  ret                           ; Returns back to the C code!

;; global jump_usermode
;; extern test_user_function
;; jump_usermode:
;;      mov ax,0x23
;;      mov ds,ax
;;      mov es,ax
;;      mov fs,ax
;;      mov gs,ax ; we don't need to worry about SS. it's handled by iret

;;      mov eax,esp
;;      push 0x23 ; user data segment with bottom 2 bits set for ring 3
;;      push eax ; push our current stack just for the heck of it
;;      pushf
;;      push 0x1B ; user code segment with bottom 2 bits set for ring 3
;;      push test_user_function
;;      iret
