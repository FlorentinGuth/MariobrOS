global enable_paging
global paging_set_4mb
extern PAGE_DIRECTORY_LOCATION  ; The Page Directory Table is defined in paging.c

section .bss
pdt:


section .text
enable_paging:
  mov eax, [PAGE_DIRECTORY_LOCATION]
  mov cr3, eax                  ; Writes the address of the page directory table to cr3

  mov ebx, cr0                  ; Reads current cr0
  or  ebx, 0x80000000           ; Sets Paging Enable bit to 1
  mov cr0, ebx                  ; Updates cr0

  ret

paging_set_4mb:
  mov ebx, cr4                  ; Reads current cr4
  or  ebx, 0x00000010           ; Sets Page Size Extensions bit to 1 (pages of 4MB instead of 4KB)
  mov cr4, ebx                  ; Updates cr4

  ret
