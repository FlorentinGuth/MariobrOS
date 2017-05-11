#include "process.h"
#include "paging.h"
#include "error.h"
#include "malloc.h"
#include "logging.h"


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
  ctx.regs = NULL;
  ctx.esp = NULL;                                  /* Will be set-up by the switching code */

  proc.context = ctx;

  return proc;
}
