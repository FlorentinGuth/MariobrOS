#include "idt.h"
#include "logging.h"

/* Source : http://www.osdever.net/bkerndev/Docs/idt.htm */


/* Declare an IDT of 256 entries.
 * If any undefined IDT entry is hit, it normally will cause an "Unhandled
 * Interrupt" exception. Any descriptor for which the 'presence' bit is cleared
 * (0) will generate an "Unhandled Interrupt" exception */
idt_entry_t idt[256];
idt_ptr idtp;

/* Use this function to set an entry in the IDT. */
void idt_set_gate(u_int8 num, u_int32 address, u_int16 selector, u_int8 privilege)
{
  idt[num].address_low = address & 0xFFFF;
  idt[num].address_high = (address >> 16) & 0xFFFF;
  idt[num].always0 = 0;
  idt[num].selector = selector;
  idt[num].gate_type = 0xE;
  idt[num].storage = 0;
  idt[num].dpl = privilege;
  idt[num].present = TRUE;
}

/* Installs and loads the IDT */
void idt_install()
{
  /* Sets the special IDT pointer up, just like in 'gdt.c' */
  idtp.limit = (sizeof (struct idt_entry) * 256) - 1;
  idtp.base = (unsigned int) &idt;

  /* Clear out the entire IDT, initializing it to zeros */
  for (int i = 0; i < 256; i++) {
    idt_set_gate(i, 0, 0, 0);
  }

  /* Add any new ISRs to the IDT here using idt_set_gate */

  /* Points the processor's internal register to the new IDT */
  idt_load();

  /* kloug(100, "IDT installed\n"); */
}
