global outb                     ; Make the function visble outside this file

outb:
  mov al, [esp + 8]             ; Data byte into al
  mov dx, [esp + 4]             ; Address of I/O port into dx
  out dx, al                    ; Sends the data
  ret
