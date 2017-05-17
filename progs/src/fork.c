#include "lib.h"

int main()
{
  u_int32 a = 0;
  u_int32 pid;
  switch (fork(1, &pid)) {
  case 0:
    printf("The fork failed\n");
    break;

  case 1:
    printf("Parent, pid of the child is %d\n", pid);
    u_int32 return_value;
    scwait(&pid, &return_value);
    printf("He (pid %d) returned %d, a = %d\n", pid, return_value, a);
    break;

  case 2:
    a = 1;
    printf("Child, pid of the parent is %d, a = %d\n", pid, a);
    exit(42);
  }

  return 0;
}
