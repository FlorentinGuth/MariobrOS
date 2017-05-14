#include "lib.h"


u_int32 syscall_fork(u_int32 priority, u_int32 *pid)
{
  u_int32 ret;
  asm volatile ("\
mov $1, %%eax;                                  \
mov %0, %%ebx;                                  \
int $0x80;                                        \
" : : "r" (priority));

  asm volatile ("mov %%eax, %0" : "=r" (ret));
  if (ret) {
    asm volatile ("mov %%ebx, %0" : "=r" (*pid));
  }
  return ret;
}

void syscall_exit(u_int32 return_value)
{
  asm volatile ("\
mov $0, %%eax;                                  \
mov %0, %%ebx;                                  \
int $0x80;                                        \
" : : "r" (return_value));
}

bool syscall_wait(u_int32 *pid, u_int32 *return_value)
{
  u_int32 ret;
  asm volatile ("\
mov $2, %eax;    \
int $0x80;         \
");

  asm volatile ("mov %%eax, %0" : "=r" (ret));
  if (ret) {
    asm volatile ("\
mov %%ebx, %0;                                              \
mov %%ecx, %1;                                              \
" : "=r" (*pid), "=r" (*return_value) : : "ebx", "ecx");
  }
  return ret;
}


void syscall_printf(string s, ...)
{
  asm volatile ("\
mov $3, %%eax;    \
mov %0, %%ebx;    \
int $0x80;          \
" : : "r" (s) : "eax", "ebx");
}
