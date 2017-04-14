/** types.h:
 *  Provides useful type and constants definitions.
 */

#ifndef TYPES_H
#define TYPES_H


/* Constants defining the memory layout */
/* The memory  is laid out as follows:
 *  - from 0MB to 1MB (0x10000), we have BIOS and GRUB-reserved memory (such as the framebuffer)
 *  - from 1MB to 8MB there is the kernel code, stack and heap (waste of space...)
 *  - from 8MB to ??? there is the user space
 */

/* The framebuffer location in memory */
#define FRAMEBUFFER_LOCATION    0x000B8000

/* A variable who is stored at the end of kernel space */
extern unsigned int ld_end;
#define END_OF_KERNEL_LOCATION  &ld_end

#define END_OF_KERNEL_HEAP      0x00800000


/* This defines what the stack looks like after an ISR was running */
struct regs
{
  unsigned int gs, fs, es, ds;                          /* pushed the segs last */
  unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pushad' */
  unsigned int int_no, err_code;                        /* our 'push byte #' and ecodes do this */
  unsigned int eip, cs, eflags, useresp, ss;            /* pushed by the processor automatically */
};
typedef struct regs regs_t;


typedef unsigned int   u_int32;
typedef   signed int   s_int32;
typedef unsigned short u_int16;
typedef   signed short s_int16;
typedef unsigned char  u_int8;
typedef   signed char  s_int8;

typedef u_int32 size_t;  /* A type to hold a number of bytes */


typedef unsigned char bool;
#define FALSE (bool)0
#define TRUE  (bool)1


/** port_t:
 *  The type of an I/O port.
 */
typedef unsigned short port_t;


#endif
