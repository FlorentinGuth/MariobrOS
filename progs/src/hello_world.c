#include "lib.h"

int main()
{
  syscall_printf("Hello World!\n");
  syscall_printf("%d %f%x%f\n", 2, 12, 0xDEADBEEF, 15);
  return 0;
}
