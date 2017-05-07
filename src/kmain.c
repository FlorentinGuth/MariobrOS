#include "kmain.h"
#include "shell.h"

/** kmain.c
 *  Contains the kernel main function.
 */

u_int32 UPPER_MEMORY;
u_int32 LOWER_MEMORY;

int kmain(multiboot_info_t* mbd)
{
  kloug(100, "Successfully booted\n");

  /* Setting the memory limits (which are given in number of 1024 bytes) */
  LOWER_MEMORY = 1024 * mbd->mem_lower;
  UPPER_MEMORY = 1024 * mbd->mem_upper;
  kloug(100, "Lower memory: %x, upper memory: %x\n", LOWER_MEMORY, UPPER_MEMORY);

  /* Installing everything */
  gdt_install();
  init_pic();
  /* timer_install(); */
  keyboard_install(TRUE);

  /* isr_install_handler(6, illegal_opcode_handler); */
  idt_install();
  isrs_install();
  irq_install();

  /* First things first: memory */
  paging_enabled = FALSE;
  malloc_install();
  paging_install();

  /* Enables interruptions */
  __asm__ __volatile__ ("sti");


  clear();
  shell_install();

  filesystem_install();

  for(;;)
    __asm__ __volatile__("hlt"); // idle state, still reacts to interrupts
  return 0xCAFEBABE;
}
