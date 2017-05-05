#include "io.h"

/* Modified versions of in and out.
 * I don't know why those work better, but, alas, they do! */


u_int8 inb (u_int16 port)
{
    u_int8 rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (port));
    return rv;
}

u_int16 inw (u_int16 port)
{
    u_int16 rv;
    __asm__ __volatile__ ("inw %1, %0" : "=a" (rv) : "dN" (port));
    return rv;
}


void outb (u_int16 port, u_int8 data)
{
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (port), "a" (data));
}

void outw (u_int16 port, u_int16 data)
{
    __asm__ __volatile__ ("outw %1, %0" : : "dN" (port), "a" (data));
}

void io_wait()
{
    /* Port 0x80 is used for 'checkpoints' during POST. */
    /* The Linux kernel seems to think it is free for use :-/ */
    asm volatile ( "outb %%al, $0x80" : : "a"(0) );
}
