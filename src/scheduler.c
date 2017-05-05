#include "scheduler.h"
#include "malloc.h"
#include "queue.h"
#include "timer.h"
#include "irq.h"
#include "syscall.h"  /* Ugly */
#include "paging.h"


scheduler_state_t *state = NULL;


void select_new_process()
{
  /* Search for a runnable process */
  bool found = FALSE;
  queue_t *temp = empty_queue();  /* The queue to temporary save processes into */

  for (priority prio = MAX_PRIORITY; prio >= 0 && !found; prio--) {
    queue_t *q = state->runqueues[prio];

    while (!is_empty_queue(q) && !found) {
      pid pid = dequeue(q);
      enqueue(temp, pid);

      if (state->processes[pid].state == Runnable) {
        /* We found a runnable process */
        found = TRUE;
        state->curr_pid = pid;
      }
    }

    /* Restoring the runqueue */
    while (!is_empty_queue(temp)) {
      enqueue(q, dequeue(temp));
    }
    /* The selected process is at the end of his runqueue now */
  }

  mem_free(temp);
}


/* Context-switching defines, not functions, to avoid stack using */
#define SWITCH_BEFORE() {                                               \
    /* Restores kernel paging */                                        \
    switch_page_directory(kernel_directory);                            \
    context_t *ctx = &state->processes[state->curr_pid].context;        \
                                                                        \
    /* Saves process context */                                         \
    asm volatile ("mov %%esp, %0" : "=r" (ctx->esp));                   \
    ctx->regs = regs;                                                   \
    ctx->first_free_block = first_free_block;                           \
    ctx->unallocated_mem  = unallocated_mem;                            \
                                                                        \
    /* Restores kernel context */                                       \
    unallocated_mem  = kernel_context.unallocated_mem;                  \
    first_free_block = kernel_context.unallocated_mem;                  \
  }

#define SWITCH_AFTER() {                                                \
    /* Saves kernel context */                                          \
    kernel_context.unallocated_mem  = unallocated_mem;                  \
    kernel_context.first_free_block = first_free_block;                 \
                                                                        \
    /* Restores process context */                                      \
    context_t *ctx = &state->processes[state->curr_pid].context;        \
    first_free_block = ctx->first_free_block;                           \
    unallocated_mem  = ctx->unallocated_mem;                            \
    /* No need to touch at the regs structure because of pointers */    \
    asm volatile ("mov %0, %%esp" : : "r" (ctx->esp));                  \
                                                                        \
    /* Restores process paging */                                       \
    switch_page_directory(ctx->page_dir);                               \
  }

void timer_handler(regs_t *regs)
{
  SWITCH_BEFORE();       /* Save context + kernel paging */
  select_new_process();
  SWITCH_AFTER();        /* Paging set-up + restore the stack and the context */
}

/**
 * @name syscall_handler - Handler for the syscall interruption
 * Decodes the syscall (in eax) and performs it on the current state.
 * @param regs           - Context of the current process
 * @return void
 */
void syscall_handler(regs_t *regs)
{
  SWITCH_BEFORE();  /* Save context + kernel paging */

  switch (regs->eax) { /* Syscall number */

  case Exit:
    syscall_exit();
    break;

  case Fork:
    syscall_fork();
    break;

  case Wait:
    syscall_wait();
    break;

  default:
    syscall_invalid();
  }

  /* Check if the syscall has not ended, and if it is the case select a new process */
  if (state->processes[state->curr_pid].state != Runnable) {
    select_new_process();
  }

  SWITCH_AFTER();  /* Paging set-up + restore the stack and the context */
}


void init()
{
  state = mem_alloc(sizeof(scheduler_state_t));

  /* Creating idle process */
  pid idle_pid = 0;
  process_t *idle = &state->processes[idle_pid];
  idle->state = Runnable;
  idle->parent_id = idle_pid;    /* Idle is its own parent */
  idle->prio = 0;
  /* TODO: context and paging directory */
  /* Idle code: for (;;) { asm("hlt"; )}; */

  /* Creating init process */
  pid init_pid = 1;
  process_t *init = &(state->processes[init_pid]);
  init->state = Runnable;
  init->parent_id = init_pid;    /* Init is its own parent */
  init->prio = MAX_PRIORITY;
  /* TODO: context and paging directory */
  /* Init code: for (;;) { syscall_wait() }; */

  /* Initialization of the state */
  state->curr_pid = init_pid;  /* We start with the init process */
  for (priority prio = 0; prio <= MAX_PRIORITY; prio++) {
    state->runqueues[prio] = empty_queue();
  }
  /* Adds idle and init in the runqueues */
  enqueue(state->runqueues[0],            idle_pid);
  enqueue(state->runqueues[MAX_PRIORITY], init_pid);

  /* Adds handlers for timer and syscall interruptions */
  extern void *timer_phase(int hz);  /* Defined in timer.c */
  timer_phase(SWITCH_FREQ);
  irq_install_handler(0, timer_handler);
  isr_install_handler(SYSCALL_ISR, syscall_handler);
}
