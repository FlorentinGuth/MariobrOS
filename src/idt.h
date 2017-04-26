#ifndef IDT_H
#define IDT_H

#include "types.h"

/*
 * Bit:     | 7 | 6 5 | 4 |  3     0  |
 * Content: | P | DPL | S | Gate type |
 * Value:   | ? | ? ? | 0 |  1 1 1 0  | = 0x?7 */

/* Defines an IDT entry */
typedef struct idt_entry idt_e;
struct idt_entry
{
  u_int16 base_lo;
  u_int16 sel;        /* Our kernel segment goes here */
  u_int8  always0;    /* This will ALWAYS be set to 0 */
  u_int8  flags;      /* Set using the above table */
  u_int16 base_hi;
} __attribute__((packed));

typedef struct idt_ptr idt_ptr;
struct idt_ptr
{
  unsigned short limit;
  unsigned int base;
} __attribute__((packed));

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);
void idt_install();

#endif
