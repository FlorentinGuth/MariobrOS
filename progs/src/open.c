#include "lib.h"


int main()
{
  fd f =syscall_open("test2/autre", 0xde, 0xbeef);
  syscall_printf("Opened\n");
  syscall_close(f);
  syscall_printf("Closed\n");
  return 0;
}
