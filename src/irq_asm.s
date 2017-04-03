%macro interrupt_request_handler 1
global irq%1
        
irq%1:
        push dword 0            ; push 0 as error code
        push dword (32+%1)      ; push the interrupt number
        jmp common_irq_handler
%endmacro

extern irq_handler


; This is a stub that we have created for IRQ based ISRs. This calls
; 'irq_handler' in 'irq.c'.
common_irq_handler:
        pushad
        push ds
        push  es
        push fs
        push gs
        mov ax, 0x10
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        mov eax, esp
        push eax
        mov eax, irq_handler
        call eax
        pop eax
        pop gs
        pop fs
        pop es
        pop ds
        popad
        add esp, 8
        iret

;;  creation of the handler
interrupt_request_handler 0
interrupt_request_handler 1
interrupt_request_handler 2
interrupt_request_handler 3
interrupt_request_handler 4
interrupt_request_handler 5
interrupt_request_handler 6
interrupt_request_handler 7
interrupt_request_handler 8
interrupt_request_handler 9
interrupt_request_handler 10
interrupt_request_handler 11
interrupt_request_handler 12
interrupt_request_handler 13
interrupt_request_handler 14
interrupt_request_handler 15
        
