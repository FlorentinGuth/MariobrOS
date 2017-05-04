#include "process.h"
#include "paging.h"
#include "error.h"


process_t new_process(pid parent_id, priority prio)
{
  process_t proc;

  proc.state = Runnable;

  proc.parent_id = parent_id;
  proc.prio = prio;

  context_t ctx;
  /* TODO */
  proc.context = ctx;

  return proc;
}
