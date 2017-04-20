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

  writef("Pour information, le symbole %c est le caractère %d\n", 47,47);
  writef("-1 : %d\n", (u_int32)(-1));
  
  paging_install();
  /* malloc_install(); */

  clear(); /* Empties the framebuffer */

  writef("Test test TEST !");
  
  for(;;)
    __asm__ __volatile__("hlt"); // idle state, still reacts to interrupts
  return 0xCAFEBABE;
}
