extern syscall_handler

global common_interrupt_handler
common_interrupt_handler:
  cli                           ; Disable interrupts

  push dword 0                  ; Error code 0
  push dword 0x80               ; Push the interrupt number

  pushad                        ; Push the 8 general purpose registers

  push ds
  push es
  push fs
  push gs

  mov ax, 0x10
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  mov eax, esp
  push eax

  mov eax, syscall_handler
  call eax

  pop eax

  pop gs
  pop fs
  pop es
  pop ds

  popad

  add esp, 8

  iret
