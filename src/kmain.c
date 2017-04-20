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

  clear(); /* Empties the framebuffer */
  
  timer_install();
  keyboard_install();

  writef("Jusqu'ici, tout va bien...\n");
  
  paging_install();
  malloc_install();
  
  writef("Test test TEST !");
  
  for(;;)
    __asm__ __volatile__("hlt"); // idle state, still reacts to interrupts
  return 0xCAFEBABE;
}
