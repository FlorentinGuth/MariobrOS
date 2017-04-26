#include "types.h"

void *memcpy(void *dst, void *src, u_int32 n)
{
    u_int8 *p = dst;
    while(n--)
        *(u_int8*)dst++ = *(u_int8*)src++;
    return p;
}
