global malloc
global free
global open
global close
global printf    

exit:
    mov $0, %eax
    mov $4(%esp), %ebx
    int $0x80
    ret

fork:
    mov $1, %eax
    mov $4(%esp), %ebx
    int $0x80
    mov $4(%esp), %edi
    mov %ebx, (%edi)
    ret

wait:
    mov $2, %eax
    int $0x80
    mov $4(%esp), %edi
    mov %ebx, (%edi)
    mov $8(%esp), %edi
    mov %ecx, (%edi)
    ret

printf:
    mov $3, %eax
    mov $4(%esp), %ebx
    int $0x80
    ret

malloc:
    mov $4, %eax
    mov $4(%esp), %ebx
    int $0x80
    ret

free:
    mov $5, %eax
    mov $4(%esp), %ebx
    int $0x80
    ret

hlt:
    mov $11, %eax
    int $0x80

open:
    mov $15, %eax
    mov $4(%esp), %ebx
    mov $8(%esp), %ecx
    and $0xff, %ecx
    mov $12(%esp), %edx
    and $0xffff, %edx
    int $0x80
    ret

close:
    mov $16, %eax
    mov $4(%esp), %ebx
    int $0x80
    ret

read:
    mov $17, %eax
    mov $4(%esp), %ebx
    mov $8(%esp), %ecx
    mov $16(%esp), %edx
    mov 20(%esp), %edi
    int $0x80
    ret

read:
    mov $18, %eax
    mov $4(%esp), %ebx
    mov $8(%esp), %ecx
    mov $16(%esp), %edx
    mov 20(%esp), %edi
    int $0x80
    ret


    
