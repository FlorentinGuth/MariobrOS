section .text                   ; Code section
align 4                         ; 4-byte aligned code

global loader                   ; The entry symbol for ELF
extern main                     ; The user main function
extern syscall_exit

loader:                         ; Entry point defined in link_prog.ld
  ; Push eventual arguments to the stack, from last to first
  ; None for the moment

  call main

  ; Now we add an exit syscall to be sure the program exits properly
  push eax                      ; Return value
  call syscall_exit
