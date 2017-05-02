#include "syscall.h"

/* Possible speed enhancements:
 * - List of child processes for each process
 * - Stack of free processes
 */

void syscall_fork(scheduler_state_t *state, priority child_prio)
{
  /* Research of a free process */
  pid id = 0;
  while (id < NUM_PROCESSES && state->processes[id].state != Free) {
    id += 1;
  }

  /* One cannot create a child process with a higher priority than its own */
  if (id == NUM_PROCESSES || child_prio > state->processes[state->curr_pid].prio) {
    asm volatile ("mov %%eax, $0");
    return;
  }

  /* Initialization of fields, registers, copying of context */
  process_t *proc = &state->processes[id];
  proc->parent_id = state->curr_pid;
  proc->state = Runnable;
  proc->context.eax = 2;
  proc->context.ebx = state->curr_pid;
  /* TODO: copy context */

  /* Adding the process in the runqueue */
  enqueue(state->runqueues[child_prio], id);

  /* Setting the values of the parent process */
  asm volatile ("mov %%eax, $1");
  asm volatile ("mov %%ebx, %0" : : "r" (id));
}

/**
 * @name resolve_exit_wait - Resolves an exit or wait syscall
 * @param state            - The scheduler state
 * @param parent           - The parent process, in waiting mode
 * @param child            - The child process, in zombie mode
 * @param return_value     - The return value of the child process
 * @return void
 */
void resolve_exit_wait(scheduler_state_t *state, pid parent, pid child, u_int32 return_value)
{
  /* Freeing the child from zombie state */
  state->processes[child].state= Free;

  /* Goodbye cruel world: removes the child from the runqueue */
  priority prio = state->processes[child].prio;
  queue_t *q = state->runqueues[prio];
  queue_t *temp = empty_queue();
  while (!is_empty_queue(q)) {
    pid pid = dequeue(q);
    if (pid != child) {
      enqueue(temp, pid);
    }
  }
  state->runqueues[prio] = temp;

  /* Notifies the parent */
  process_t* parent_proc = &state->processes[parent];
  parent_proc->state= Runnable;
  parent_proc->context.eax = 1;
  parent_proc->context.ebx = child;
  parent_proc->context.ecx = return_value;
}

void syscall_exit(scheduler_state_t *state, u_int32 return_value)
{
  pid id = state->curr_pid;
  state->processes[id].state= Zombie;

  /* Notifies the child processes of the exiting one */
  for (pid i = 0; i < NUM_PROCESSES; i++) {
    if (state->processes[i].parent_id == id) {
      state->processes[i].parent_id = 1;
    }
  }

  /* Checks whether the parent was waiting for us to die (how cruel!) */
  pid parent_id = state->processes[id].parent_id;
  if (state->processes[parent_id].state == Waiting) {
    resolve_exit_wait(state, parent_id, id, return_value);
  }
}

void syscall_wait(scheduler_state_t *state)
{
  state->processes[state->curr_pid].state = Waiting;
  pid parent_id = state->curr_pid;
  bool has_children = FALSE;

  for (pid id = 0; id < NUM_PROCESSES; id++) {
    process_t proc = state->processes[id];
    if (proc.parent_id == parent_id) {
      has_children = TRUE;
      if (proc.state== Zombie) {
        u_int32 return_value = proc.context.ebx;  /* TODO: be sure of this */
        resolve_exit_wait(state, parent_id, id, return_value);
        return;
      }
    }
  }

  if (!has_children) {
    /* The process has no children, the call terminates instantly */
    asm volatile ("mov %%eax, $0");
    state->processes[state->curr_pid].state = Runnable;
  }
}
