#include "process.h"
#include "paging.h"


void transfer_control(process_t *proc)
{
  if (proc->state != Runnable) {
    return;
  }

  /* Paging set-up */
  switch_page_directory(proc->page_dir);

  /* User-mode code */
  /* I trust it! */

  /* Restores context */
  asm volatile ("add $4, %esp"); /* Ignores the return address of the callee */
  asm volatile ("pop %gs");
  asm volatile ("pop %fs");
  asm volatile ("pop %es");
  asm volatile ("pop %ds");
  asm volatile ("popal");         /* Equivalent of popad (Intel) in AT&T syntax */
  asm volatile ("add $8, %esp");  /* Ignores int_no and err_code */
  asm volatile ("iret");          /* Jump to eip using code and stack segments, and user esp */
}
