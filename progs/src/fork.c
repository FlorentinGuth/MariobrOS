#include "lib.h"

int main()
{
  u_int32 pid;
  switch (fork(1, &pid)) {
  case 0:
    printf("The fork failed\n");
    break;

  case 1:
    printf("Parent, pid of the child is %d\n", pid);
    u_int32 return_value;
    scwait(&pid, &return_value);
    printf("He (%d) returned %d\n", pid, return_value);
    break;

  case 2:
    printf("Child, pid of the parent is %d\n", pid);
    exit(42);
  }

  return 0;
}
