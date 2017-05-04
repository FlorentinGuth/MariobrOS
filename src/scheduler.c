#include "scheduler.h"
#include "malloc.h"
#include "queue.h"
#include "timer.h"
#include "irq.h"
#include "syscall.h"  /* Ugly */


scheduler_state_t *state = NULL;


/* Timer handler */
void timer_handler(regs_t *regs);


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

void timer_handler(regs_t *regs)
{
  /* Save context */
  state->processes[state->curr_pid].context = *regs;

  select_new_process();

  transfer_control(&state->processes[state->curr_pid]);
}
