#include "kmain.h"
#include "paging.h"
#include "malloc.h"

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

  for(unsigned int c = 128; c<256; c++) {
    write_int(c);write_char('=');write_char(c); write_string(",\t");
  }
  write_char('\n');write_char('\n');

  write_string("La plupart des accents courants sont désormais gérés.\nVoici les accents qui existent : éèêë É(ÈÊË) à(À) ù(Ù) ô(Ô) çÇ\nEntre parenthèses, ceux qui n'existent pas (cf tableau)\n");


  paging_install();

  clear(); /* Empties the framebuffer */


  write_string("Installing malloc...\n");
  malloc_install();
  write_string("Malloc installed!\n");

  void *a = mem_alloc(0x700000);
  write_hex((u_int32)a);
  void *b = mem_alloc(0x700000);
  write_hex((u_int32)b);
  mem_free(a);
  void *c = mem_alloc(0x700000);
  write_hex((u_int32)c);
  void *d = mem_alloc(1);
  write_hex((u_int32)d);
  mem_free(c);
  mem_free(d);
  void *e = mem_alloc(0x700000);
  write_hex((u_int32)e);

  for(;;)
    __asm__ __volatile__("hlt"); // idle state, still reacts to interrupts
  return 0xCAFEBABE;
}
