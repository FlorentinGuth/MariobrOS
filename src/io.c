#include "io.h"

/* Modified versions of in and out.
 * I don't know why those work better, but, alas, they do! */


unsigned char inb (unsigned short port)
{
    unsigned char rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (port));
    return rv;
}

unsigned char inw (unsigned short port)
{
    unsigned int rv;
    __asm__ __volatile__ ("inw %1, %w0" : "=a" (rv) : "dN" (port));
    return rv;
}


void outb (unsigned short port, unsigned char data)
{
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (port), "a" (data));
}

void outw (unsigned short port, unsigned int data)
{
    __asm__ __volatile__ ("outw %w1, %0" : : "dN" (port), "a" (data));
}
