#include "process.h"
#include "paging.h"
#include "error.h"
#include "malloc.h"
#include "logging.h"
#include "gdt.h"


process_t new_process(pid parent_id, priority prio)
{
  kloug(100, "Creating new process\n");

  process_t proc;
  proc.state = Runnable;
  proc.parent_id = parent_id;
  proc.prio = prio;

  context_t ctx;
  ctx.page_dir = new_page_dir(&ctx.first_free_block, &ctx.unallocated_mem);
  kloug(100, "Malloc state: %x %x\n", ctx.first_free_block, ctx.unallocated_mem);

  regs_t *regs = (regs_t *)mem_alloc(sizeof(regs_t));
  /* The data and general purpose segment registers are set to the user data segment */
  regs->ds = regs->es = regs->fs = regs->gs = USER_DATA_SEGMENT | 0x3;
  /* All general purpose registers are set to 0 */
  regs->eax = regs->ebx = regs->ecx = regs->edx = regs->esi = regs->edi = regs->esp = regs->ebp = 0;
  /* We don't care about the error codes */
  regs->int_no = regs->err_code= 0;
  /* There comes the interesting stuff */
  regs->eip = NULL;  /* Will be set during code loading */
  regs->cs = USER_CODE_SEGMENT | 0x3;
  regs->eflags = 0;
  regs->useresp = START_OF_USER_STACK;
  regs->ss = USER_DATA_SEGMENT | 0x3;

  ctx.regs = regs;
  ctx.esp = NULL;                                  /* Will be set-up by the switching code */
  proc.context = ctx;

  return proc;
}
