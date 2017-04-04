/** types.h:
 *  Provides useful type definitions.
 */

#ifndef TYPES_H
#define TYPES_H


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
