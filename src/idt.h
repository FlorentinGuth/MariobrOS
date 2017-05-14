#ifndef IDT_H
#define IDT_H

#include "idt_asm.h"
#include "types.h"

/* Defines an IDT entry */
typedef struct idt_entry
{
  u_int16 address_low  : 16; /* Lower part of the handler address */
  u_int16 selector     : 16; /* Code selector of the interrupt function:
                              * to generate this interrupt, cpl should be <= dpl */
  u_int8  always0      :  8; /* This will ALWAYS be set to 0 */
  u_int8  gate_type    :  4; /* Possible values in 32 bit:
                              * 0x5: task gate
                              * 0xE: interrupt gate
                              * 0xF: trap gate
                              */
  bool    storage      :  1; /* 0 for interrupt gates?? */
  u_int8  dpl          :  2; /* Descriptor Privilege Level, if 0 restricts the interruption to the kernel */
  bool    present      :  1; /* Whether this is a real IDT entry and not random data */
  u_int16 address_high : 16; /* Higher part of the address */
} __attribute__((packed)) idt_entry_t;
/** Different types of gates:
 *  - an interrupt gate specifies an ISR (uses cs, then iret), disable interrupts, storage = 0
 *  - a trap gate is exactly the same as ISRs, except that they do not disabe interrupts
 *  - a task gate is used for task switches (with the TSS)
 */

typedef struct idt_ptr idt_ptr;
struct idt_ptr
{
  unsigned short limit;
  unsigned int base;
} __attribute__((packed));

void idt_set_gate(u_int8 num, u_int32 address, u_int16 selector, u_int8 privilege);
void idt_install();

#endif
