#include "memory.h"


void mem_set(void* str, u_int8 c, size_t len)
{
  u_int8 *ptr = (u_int8 *)str;
  for (size_t i = 0; i < len; i++) {
    *ptr = c;
    ptr++;
  }
}

void mem_copy(void *dest, const void *source, size_t len)
{
  u_int8 *ptr_s = (u_int8 *)source;
  u_int8 *ptr_d = (u_int8 *)dest;
  for (size_t i = 0; i < len; i++) {
    *ptr_d = *ptr_s;
    ptr_d++;
    ptr_s++;
    /* Or, if you'd like it put more simply, *(ptr_d++) = *(ptr_s++); */
  }
}
