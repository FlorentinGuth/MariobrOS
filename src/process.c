#include "process.h"
#include "paging.h"


void transfer_control(process_t *proc)
{
  if (proc->state != Runnable)
    return;

  /* Paging set-up */
  switch_page_directory(proc->page_dir);

  /* User-mode code */
  /* I trust him! */

  /* Restores context */
  /* TODO */
}
