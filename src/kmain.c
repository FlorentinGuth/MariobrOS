#include "kmain.h"
#include "paging.h"
#include "malloc.h"

/** kmain.c
 *  Contains the kernel main function.
 */

u_int32 UPPER_MEMORY;
u_int32 LOWER_MEMORY;

int kmain(multiboot_info_t* mbd)
{
  log_string("Successfully booted\n", Info);

  /* Setting the memory limits (which are given in number of 1024 bytes) */
  LOWER_MEMORY = 1024 * mbd->mem_lower;
  UPPER_MEMORY = 1024 * mbd->mem_upper;

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

  void *a = mem_alloc(4);
  writef("%x\n", a);

  void *b = mem_alloc(0x1000);
  writef("%x\n", b);

  void *c = mem_alloc(0xF000);
  writef("%x\n", c);

  void *d = mem_alloc(0xFB4 - 2*3*4);  /* Fills last page */
  writef("%x\n", d);

  void *e = mem_alloc(1);
  writef("%x\n", e);
  write_memory();

  for(;;)
    __asm__ __volatile__("hlt"); // idle state, still reacts to interrupts
  return 0xCAFEBABE;
}
