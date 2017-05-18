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
  mov edi, [esp+16]
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

global ls
ls:
    push ebx
    push ecx
    mov eax, 6
    mov ebx, [esp+12]
    mov ecx, [esp+16]
    int 0x80
    pop ecx
    pop ebx
    ret

global rm
rm:
    push ebx
    push ecx
    push edx
    mov eax, 7
    mov ebx, [esp+16]
    mov ecx, [esp+20]
    mov edx, [esp+24]
    int 0x80
    pop edx
    pop ecx
    pop ebx
    ret

global mkdir
mkdir:
    push ebx
    push ecx
    mov eax, 8
    mov ebx, [esp+12]
    mov ecx, [esp+16]
    int 0x80
    pop ecx
    pop ebx
    ret

global keyget
keyget:
    mov eax, 9
    int 0x80
    ret

global run_program
run_program:
    push ebx
    mov eax, 10
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
  pop edx
  pop ecx
  pop ebx
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

global writef
writef:
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

global lseek
lseek:
    push ebx
    push ecx
    push edx
    mov eax, 19
    mov ebx, [esp+16]
    mov ecx, [esp+20]
    mov edx, [esp+24]
    int 0x80
    pop edx
    pop ecx
    pop ebx
    ret

global fstat
fstat:
    push ebx
    push ecx
    mov eax, 20
    mov ebx, [esp+12]
    mov ecx, [esp+16]
    int 0x80
    pop ecx
    pop ebx
    ret

global set_cursor_pos
set_cursor_pos:
    push ebx
    mov ebx, [esp+8]
    mov eax, 21
    int 0x80
    pop ebx
    ret
    
global get_cursor_pos
get_cursor_pos:
    mov eax, 22
    int 0x80
    ret

global set_char
set_char:
    push ebx
    push ecx
    mov eax, 23
    mov ebx, [esp+12]
    mov ecx, [esp+16]
    int 0x80
    pop ecx
    pop ebx
    ret

global get_char
get_char:
    push ebx
    mov eax, 24
    mov ebx, [esp+8]
    int 0x80
    pop ebx
    ret

global get_cwd
get_cwd:
    push ebx
    push ecx
    mov eax, 25
    mov ebx, [esp+12]
    mov ecx, [esp+16]
    int 0x80
    pop ecx
    pop ebx
    ret

global find_dir
find_dir:   
    push ebx
    push ecx
    mov eax, 26
    mov ebx, [esp+12]
    mov ecx, [esp+16]
    int 0x80
    pop ecx
    pop ebx
    ret

global scroll
scroll:
    mov eax, 29
    int 0x80
    ret
