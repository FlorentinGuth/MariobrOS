/** types.h:
 *  Provides useful type and constants definitions.
 */

#ifndef TYPES_H
#define TYPES_H


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

#define NULL  0


/* Constants defining the memory layout */
/* The memory  is laid out as follows:
 *  - from LOWER_MEMORY to 1MB (0x10000), we have BIOS and GRUB-reserved memory (such as the framebuffer)
 *  - from 1MB to END_OF_KERNEL_LOCATION there is the kernel code, data and stack
 *  - from END_OF_KERNEL_LOCATION to ??? there is the kernel heap
 *  - from START_OF_USER_HEAP to ??? there is the user heap (starts after a bit of the kernel heap,
 *    to be able to access its page directory and such things)
 *  - from ??? to START_OF_USER_STACK there is the user stack
 *  - from START_OF_USER_CODE = START_OF_USER_STACK to UPPER_MEMORY there is the user code
 */

/* Physical memory - set by kmain */
extern u_int32 LOWER_MEMORY;  /* The memory from 0x0 to LOWER_MEMORY is free */
extern u_int32 UPPER_MEMORY;  /* The memory from 1MB to UPPER_MEMORY is free */

/* The framebuffer location in memory */
#define FRAMEBUFFER_LOCATION    0x000B8000

/* A variable who is stored at the end of kernel space */
extern unsigned int ld_end;
#define END_OF_KERNEL_LOCATION  &ld_end

/* Two variables set by kmain to determine the boundaries of the kernel stack */
extern u_int32 START_OF_KERNEL_STACK;  /* Beware, START > END as the stack grows downward! */
extern u_int32 END_OF_KERNEL_STACK;

/* User space - set by paging_install */
extern u_int32 START_OF_USER_HEAP;
extern u_int32 START_OF_USER_STACK;
extern u_int32 START_OF_USER_CODE;


/* This defines what the stack looks like after an ISR was running */
struct regs
{
  unsigned int gs, fs, es, ds;                          /* pushed the segs last */
  unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pushad' */
  unsigned int int_no, err_code;                        /* our 'push byte #' and ecodes do this */
  unsigned int eip, cs, eflags, useresp, ss;            /* pushed by the processor automatically */
} __attribute__((packed));
typedef struct regs regs_t;


/** port_t:
 *  The type of an I/O port.
 */
typedef unsigned short port_t;


#endif
