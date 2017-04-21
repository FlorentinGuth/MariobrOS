#include "kmain.h"
#include "paging.h"
#include "malloc.h"

/** kmain.c
 *  Contains the kernel main function.
 */


int kmain(multiboot_info_t* mbd)
{
  /* Setting the memory limits (which are given in number of 1024 bytes) */
  const u_int32 LOWER_MEMORY = 1024 * mbd->mem_lower;
  const u_int32 UPPER_MEMORY = 1024 * mbd->mem_upper;

  /* Installing everything */
  gdt_install();
  idt_install();
  isrs_install();
  irq_install();
  __asm__ __volatile__ ("sti");

  clear(); /* Empties the framebuffer */
  
  timer_install();
  keyboard_install();

  writef("LOWER_MEMORY: %x\nUPPER_MEMORY: %x\n", LOWER_MEMORY, UPPER_MEMORY);
  
  paging_install();
  malloc_install();
  
  writef("Yes ! YES !! It works !!!\n");

  for(;;)
    __asm__ __volatile__("hlt"); // idle state, still reacts to interrupts
  return 0xCAFEBABE;
}
