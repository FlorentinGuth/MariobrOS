global exit

exit:
    mov eax, 0
    mov ebx, [esp+4]
    int 0x80
    ret

global fork
    
fork:
    mov eax, 1
    mov ebx, [esp+4]
    int 0x80
    mov edi, [esp+4]
    mov [edi], ebx
    ret

global scwait
    
scwait:
    mov eax, 2
    int 0x80
    mov edi, [esp+4]
    mov [edi], ebx
    mov edi, [esp+8]
    mov [edi], ecx
    ret

global malloc
    
malloc:
    mov eax, 4
    mov ebx, [esp+4]
    int 0x80
    ret

global free
    
free:
    mov eax, 5
    mov ebx, [esp+4]
    int 0x80
    ret

global hlt
    
hlt:
    mov eax, 11
    int 0x80
    ret

global open
    
open:
    mov eax, 15
    mov ebx, [esp+4]
    mov ecx, [esp+8]
    and ecx, 0xff
    mov edx, [esp+12]
    and edx, 0xffff
    int 0x80
    ret

global close
    
close:
    mov eax, 16
    mov ebx, [esp+4]
    int 0x80
    ret

global read
    
read:
    mov eax, 17
    mov ebx, [esp+4]
    mov ecx, [esp+8]
    mov edx, [esp+12]
    mov edi, [esp+16]
    int 0x80
    ret

global write
    
write:
    mov eax, 18
    mov ebx, [esp+4]
    mov ecx, [esp+8]
    mov edx, [esp+12]
    mov edi, [esp+16]
    int 0x80
    ret

global printf
printf:
  push   ebp
  mov    ebp, esp
  push   ebx
  mov    eax, [esp]
  add    eax, 0x1e36
  mov    edx, [ebp+8]
  mov    eax, 0x3
  mov    ebx, edx
  int    0x80
  pop    ebx
  pop    ebp
  ret
