#include "kmain.h"
#include "shell.h"

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
  init_pic();

  timer_install();
  keyboard_install(TRUE);

  paging_install();
  malloc_install();
  
  idt_install();
  isrs_install();
  irq_install();
  
  __asm__ __volatile__ ("sti");
  
  clear(); /* Empties the framebuffer */
  set_disk(FALSE);
  
  /* u_int16 buffer[512]; */
  /* u_int16 original[512]; */
  /* int pos = 2; */
  /* readPIO(pos,2, original); */
  /* for(int i=150; i<300; i++) { */
  /*   writef("%x,", original[i]); */
  /*   buffer[i] = i; */
  /* } */
  /* writePIO(pos,2,buffer); */
  /* writef("\n\n"); */
  /* readPIO(pos,2,buffer); */
  /* for(int i=150; i<300; i++) { */
  /*   writef("%x,", buffer[i]); */
  /*   buffer[i] = 0; */
  /* } */

  analyze_superblock();

  for(;;)
    __asm__ __volatile__("hlt"); // idle state, still reacts to interrupts
  return 0xCAFEBABE;
}
