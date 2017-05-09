#include "process.h"
#include "paging.h"
#include "error.h"
#include "malloc.h"


process_t new_process(pid parent_id, priority prio)
{
  process_t proc;

  proc.state = Runnable;

  proc.parent_id = parent_id;
  proc.prio = prio;

  context_t ctx;
  ctx.page_dir = new_page_dir();
  malloc_new_state(ctx.page_dir, start_of_user_heap, &ctx.first_free_block, &ctx.unallocated_mem);

  ctx.regs = (regs_t *)mem_alloc(sizeof(regs_t));  /* Left uninitialized, except for esp */
  /* TODO */
  ctx.esp = NULL;                                  /* Will be set-up by the switching code */

  proc.context = ctx;

  return proc;
}
