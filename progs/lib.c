#include "lib.h"


u_int32 syscall_fork(u_int32 priority, u_int32 *pid)
{
  u_int32 ret;
  asm volatile ("mov $1, %eax");  /* Fork is syscall number 1 */
  asm volatile ("mov %0, %%ebx" : : "r" (priority));

  asm volatile ("int $20");       /* Interruption 20 is syscall */

  asm volatile ("mov %%eax, %0" : "=r" (ret));
  if (ret) {
    asm volatile ("mov %%ebx, %0" : "=r" (*pid));
  }
  return ret;
}

void syscall_exit(u_int32 return_value)
{
  asm volatile ("mov $0, %eax");  /* Exit is syscall number 0 */
  asm volatile ("mov %0, %%ebx" : : "r" (return_value));

  asm volatile ("int $20");       /* Interruption 20 is syscall */
}

bool syscall_wait(u_int32 *pid, u_int32 *return_value)
{
  u_int32 ret;
  asm volatile ("mov $2, %eax");  /* Wait is syscall number 2 */

  asm volatile ("int $20");       /* Interruption 20 is syscall */

  asm volatile ("mov %%eax, %0" : "=r" (ret));
  if (ret) {
    asm volatile ("mov %%ebx, %0" : "=r" (*pid));
    asm volatile ("mov %%ecx, %0" : "=r" (*return_value));
  }
  return ret;
}
