#include "idt.h"

/* Declare an IDT of 256 entries.
 * If any undefined IDT entry is hit, it normally will cause an "Unhandled
 * Interrupt" exception. Any descriptor for which the 'presence' bit is cleared
 * (0) will generate an "Unhandled Interrupt" exception */
idt_e idt[IDT_SIZE];
idt_ptr idtp;

/* Use this function to set an entry in the IDT. */
void idt_set_gate(u_int8 num, u_int32 base, u_int16 sel, u_int8 flags)
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
  idtp.limit = IDTSIZE*8;
  idtp.base = IDTBASE;

  /* Clear out the entire IDT, initializing it to zeros */
  for (int i=0; i<256; i++) {
    idt_set_gate(i,0,0,0);
  }

  /* Add any new ISRs to the IDT here using idt_set_gate */

  /* Points the processor's internal register to the new IDT */
  memcpy((u_int8*) idtp.base, (u_int8*) idt, idt.limit);
  __asm__ __volatile__ ("lidtl (idtr)");
}
