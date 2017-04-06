/** types.h:
 *  Provides useful type definitions.
 */

#ifndef TYPES_H
#define TYPES_H


/* Constants defining the memory layout */
/* The memory  is laid out as follows:
 *  - from 0MB to 1MB (0x10000), we have BIOS and GRUB-reserved memory (such as the framebuffer)
 *  - from 1MB to 4MB there is the kernel code, stack and heap (waste of space...)
 *  - from 4MB to 8MB there is the 1024 page tables of 1024 entries of 4 bytes each
 *  - from 8MB to ??? there is the user space
 */

/* The framebuffer location in memory */
#define FRAMEBUFFER_LOCATION    0x000B8000

#define PAGE_TABLES_LOCATION    0x00400000

/* A variable who is stored at the end of kernel space */
extern unsigned int end_of_kernel_space;
#define END_OF_KERNEL_LOCATION  &end_of_kernel_space

/* This defines what the stack looks like after an ISR was running */
struct regs
{
  unsigned int gs, fs, es, ds;                          /* pushed the segs last */
  unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pushad' */
  unsigned int int_no, err_code;                        /* our 'push byte #' and ecodes do this */
  unsigned int eip, cs, eflags, useresp, ss;            /* pushed by the processor automatically */
};


typedef unsigned char byte;

typedef unsigned char bool;
#define FALSE (bool)0
#define TRUE  (bool)1


/** port_t:
 *  The type of an I/O port.
 */
typedef unsigned short port_t;


#endif
