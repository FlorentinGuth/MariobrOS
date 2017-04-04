#ifndef IDT_H
#define IDT_H


/*
 * Bit:     | 7 | 6 5 | 4 | 3       0 |
 * Content: | P | DPL | S | Gate type |
 * Value:   | ? | ? ? | 0 |  1 1 1 0  | = 0x?7 */

/* Defines an IDT entry */
typedef struct idt_entry idt_e;
struct idt_entry
{
  unsigned short base_lo;
  unsigned short sel;        /* Our kernel segment goes here */
  unsigned char always0;     /* This will ALWAYS be set to 0 */
  unsigned char flags;       /* Set using the above table */
  unsigned short base_hi;
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
