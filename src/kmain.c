#include "kmain.h"
#include "paging.h"

/** kmain.c
 *  Contains the kernel main function.
 */


int kmain() /*struct multiboot mboot)*/
{
  gdt_install();
  idt_install();
  isrs_install();
  irq_install();
  __asm__ __volatile__ ("sti");

  timer_install();
  keyboard_install();
  paging_install();

  clear(); // Empties the framebuffer


  for(unsigned int c = 128; c<256; c++) {
    write_int(c);write_char('=');write_char(c); write_string(",\t");
  }
  write_char('\n');write_char('\n');

  write_string("La plupart des accents courants sont désormais gérés.\nVoici les accents qui existent : éèêë É(ÈÊË) à(À) ù(Ù) ô(Ô) çÇ\nEntre parenthèses, ceux qui n'existent pas (cf tableau)\n");

  for(;;)
    __asm__ __volatile__("hlt"); // idle state, still reacts to interrupts
  return 0xCAFEBABE;
}
