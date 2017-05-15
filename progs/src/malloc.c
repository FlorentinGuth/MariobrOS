#include "lib.h"


int main()
{
  void *ptrs[1000];

  for (int i = 0; i < 1000; i++)
    ptrs[i] = syscall_malloc(1);
  syscall_printf("Survived allocation\n");
  for (int i = 0; i < 1000; i++)
    syscall_free(ptrs[i]);
  syscall_printf("I survived!\n");

  return 0;
}
