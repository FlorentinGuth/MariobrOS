#include "idt.h"

/* Declare an IDT of 256 entries. 
 * If any undefined IDT entry is hit, it normally will cause an "Unhandled  
 * Interrupt" exception. Any descriptor for which the 'presence' bit is cleared
 * (0) will generate an "Unhandled Interrupt" exception */
idt_e idt[256];
idt_ptr idtp;

/* This exists in 'start.asm', and is used to load our IDT */
extern void idt_load();

/* Use this function to set an entry in the IDT. Alot simpler
*  than twiddling with the GDT ;) */

#pragma GCC diagnostic ignored "-Wunused-parameter"
void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags)
{
    /* We'll leave you to try and code this function: take the
    *  argument 'base' and split it up into a high and low 16-bits,
    *  storing them in idt[num].base_hi and base_lo. The rest of the
    *  fields that you must set in idt[num] are fairly self-
    *  explanatory when it comes to setup */
}
#pragma GCC diagnostic pop

/* Installs the IDT */
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
