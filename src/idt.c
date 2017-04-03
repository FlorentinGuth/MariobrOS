#include "idt.h"

/* Source : http://www.osdever.net/bkerndev/Docs/idt.htm */


/* Declare an IDT of 256 entries.
 * If any undefined IDT entry is hit, it normally will cause an "Unhandled
 * Interrupt" exception. Any descriptor for which the 'presence' bit is cleared
 * (0) will generate an "Unhandled Interrupt" exception */
idt_e idt[256];
idt_ptr idtp;

/* This exists in 'idt_asm.s', and is used to load our IDT */
extern void idt_load();

/* Use this function to set an entry in the IDT. */
void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags)
{
  idt[num].base_lo = base & 0xFFFF;
  idt[num].base_hi = (base >> 16) & 0xFFFF;
  idt[num].always0 = 0;
  idt[num].sel = sel;
  idt[num].flags = flags;
}

/* Installs and loads the IDT */
void idt_install()
{
  /* Sets the special IDT pointer up, just like in 'gdt.c' */
  idtp.limit = (sizeof (struct idt_entry) * 256) - 1;
  idtp.base = (unsigned int) &idt;

  /* Clear out the entire IDT, initializing it to zeros */
  for (int i=0; i<256; i++) {
    idt_set_gate(i,0,0,0);
  }

  /* Add any new ISRs to the IDT here using idt_set_gate */

  /* Points the processor's internal register to the new IDT */
  idt_load();
}
