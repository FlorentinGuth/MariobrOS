global exit
exit:
  push ebx
  mov eax, 0
  mov ebx, [esp+8]
  int 0x80
  pop ebx
  ret

global fork
fork:
  push ebx
  push edi
  mov eax, 1
  mov ebx, [esp+12]
  int 0x80
  mov edi, [esp+12]
  mov [edi], ebx
  pop edi
  pop ebx
  ret

global scwait
scwait:
  push ebx
  push ecx
  push edi
  mov eax, 2
  int 0x80
  mov edi, [esp+16]
  mov [edi], ebx
  mov edi, [esp+20]
  mov [edi], ecx
  pop edi
  pop ecx
  pop ebx
  ret

global malloc
malloc:
  push ebx
  mov eax, 4
  mov ebx, [esp+8]
  int 0x80
  pop ebx
  ret

global free
free:
  push ebx
  mov eax, 5
  mov ebx, [esp+8]
  int 0x80
  pop ebx
  ret

global hlt
hlt:
  mov eax, 11
  int 0x80
  ret

global open
open:
  push ebx
  push ecx
  push edx
  mov eax, 15
  mov ebx, [esp+16]
  mov ecx, [esp+20]
  and ecx, 0xff
  mov edx, [esp+24]
  and edx, 0xffff
  int 0x80
  push ebx
  push ecx
  push edx
  ret

global close
close:
  push ebx
  mov eax, 16
  mov ebx, [esp+8]
  int 0x80
  pop ebx
  ret

global read
read:
  push ebx
  push ecx
  push edx
  push edi
  mov eax, 17
  mov ebx, [esp+20]
  mov ecx, [esp+24]
  mov edx, [esp+28]
  mov edi, [esp+32]
  int 0x80
  pop edi
  pop edx
  pop ecx
  pop ebx
  ret

global write
write:
  push ebx
  push ecx
  push edx
  push edi
  mov eax, 18
  mov ebx, [esp+20]
  mov ecx, [esp+24]
  mov edx, [esp+28]
  mov edi, [esp+32]
  int 0x80
  pop edi
  pop edx
  pop ecx
  pop ebx
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
