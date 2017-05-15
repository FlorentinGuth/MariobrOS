#include "lib.h"

int main()
{
  u_int32 pid;
  switch (syscall_fork(1, &pid)) {
  case 0:
    syscall_printf("The fork failed\n");
    break;

  case 1:
    syscall_printf("Parent, pid of the child is %d\n", pid);
    u_int32 return_value;
    syscall_wait(&pid, &return_value);
    syscall_printf("He (%d) returned %d\n", pid, return_value);
    break;

  case 2:
    syscall_printf("Child, pid of the parent is %d\n", pid);
    syscall_exit(42);
  }

  return 0;
}
