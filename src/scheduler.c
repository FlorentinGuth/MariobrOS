#include "scheduler.h"
#include "malloc.h"


scheduler_state_t *init()
{
  scheduler_state_t *state = mem_alloc(sizeof(scheduler_state_t));

  /* Creating idle process */
  pid idle_pid = 0;
  process_t *idle = &state->processes[idle_pid];
  idle->state = Runnable;
  idle->parent_id = idle_pid;    /* Idle is its own parent */
  idle->prio = 0;
  /* TODO: context and paging directory */

  /* Creating init process */
  pid init_pid = 1;
  process_t *init = &(state->processes[init_pid]);
  init->state = Runnable;
  init->parent_id = init_pid;    /* Init is its own parent */
  init->prio = MAX_PRIORITY;
  /* TODO: context and paging directory */

  /* Initialization of the state */
  state->curr_pid = init_pid;  /* We start with the init process */
  for (priority prio = 0; prio <= MAX_PRIORITY; prio++) {
    state->runqueues[prio] = empty_queue();
  }
  /* Adds idle and init in the runqueues */
  enqueue(state->runqueues[0],            idle_pid);
  enqueue(state->runqueues[MAX_PRIORITY], init_pid);

  return state;
}


void select_new_process(scheduler_state_t *state)
{
  /* Save context */
  /* process_t *proc = &state->processes[state->curr_pid]; */
  /* TODO */

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
        state->time_slices_left = MAX_TIME_SLICES;


        /* Restore context */
        /* process_t *new_proc = &state->processes[state->curr_pid]; */
        /* TODO: switch_process() */
      }
    }

    /* Restoring the runqueue */
    while (!is_empty_queue(temp)) {
      enqueue(q, dequeue(temp));
    }
  }

  mem_free(temp);
}

void send_timer_event(scheduler_state_t *state)
{
  if (state->time_slices_left == 0) {
    select_new_process(state);
  } else {
    state->time_slices_left -= 1;
  }
}
