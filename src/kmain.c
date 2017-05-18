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


bool user_shell = TRUE;  /* TRUE for user shell, FALSE for kernel shell */

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
  /* kloug(100, "%x %x %x %x %x %x %x\n", KERNEL_CODE_SEGMENT, KERNEL_DATA_SEGMENT, KERNEL_STACK_SEGMENT, USER_CODE_SEGMENT, USER_DATA_SEGMENT, USER_STACK_SEGMENT, TSS_SEGMENT); */
  init_pic();
  /* timer_install(); */
  /* isr_install_handler(6, illegal_opcode_handler); */
  /* isr_install_handler(8, double_fault_handler); */
  /* isr_install_handler(13, gpf_handler); */
  idt_install();
  isrs_install();
  irq_install();

  filesystem_install();
  fs_inter_install();

  /* log_memory(); */

  if (user_shell) {
    keyboard_install(2);
    scheduler_install(TRUE);
  } else {
    keyboard_install(1);
    shell_install();
    scheduler_install(FALSE);
  }

  return 0xCAFEBABE;
}
