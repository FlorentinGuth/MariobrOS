#include "paging.h"
#include "gdt.h"
#include "timer.h"
#include "multiboot.h"
#include "malloc.h"
#include "fs_inter.h"
#include "shell.h"
#include "scheduler.h"

/** kmain.c
 *  Contains the kernel main function.
 */

u_int32 UPPER_MEMORY, LOWER_MEMORY;
u_int32 START_OF_KERNEL_STACK, END_OF_KERNEL_STACK;

int kmain(multiboot_info_t* mbd, u_int32 stack_start, u_int32 stack_size)
{
  kloug(100, "-----------Successfully booted----------\n");

  /* Setting the memory limits (which are given in number of 1024 bytes) */
  LOWER_MEMORY = 1024 * mbd->mem_lower;
  UPPER_MEMORY = 1024 * mbd->mem_upper;
  kloug(100, "Lower memory: %x, upper memory: %x\n", LOWER_MEMORY, UPPER_MEMORY);

  START_OF_KERNEL_STACK = stack_start;
  END_OF_KERNEL_STACK   = stack_start - stack_size;
  kloug(100, "Stack goes from %x to %x\n", START_OF_KERNEL_STACK, END_OF_KERNEL_STACK);

  /* First things first: memory */
  paging_enabled = FALSE;  /* Do not change this line */
  malloc_install();
  paging_install();

  /* Segmentation and interruptions */
  gdt_install();
  init_pic();
  /* timer_install(); */
  keyboard_install(TRUE);
  isr_install_handler(6, illegal_opcode_handler);
  isr_install_handler(8, double_fault_handler);
  isr_install_handler(13, gpf_handler);
  idt_install();
  isrs_install();
  irq_install();

  filesystem_install();
  fs_inter_install();

  /* log_memory(); */

  scheduler_install();

  /* Last but not least, the shell */
  shell_install();

  /* Enables interruptions */
  asm volatile ("sti");
  for(;;)
    asm volatile("hlt"); // idle state, still reacts to interrupts
  return 0xCAFEBABE;
}
