#include "lib.h"


int main()
{
  void *a = syscall_malloc(1);
  syscall_printf("%x\n", a);
  void *b = syscall_malloc(0x2000);
  syscall_printf("%x\n", b);
  syscall_free(a);
  syscall_free(b);
  void * c = syscall_malloc(0x200);
  syscall_printf("%x\n", c);
  syscall_free(c);

  return 0;
}
