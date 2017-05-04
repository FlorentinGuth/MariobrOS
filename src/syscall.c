#include "syscall.h"
#include "error.h"
#include "scheduler.h"
#include "queue.h"


/* Possible speed enhancements:
 * - List of child processes for each process
 * - Stack of free processes
 */

extern scheduler_state_t *state;  /* Defined in scheduler.c */

void syscall_fork()
{
  priority child_prio = state->processes[state->curr_pid].context.ebx;

  /* Research of a free process */
  pid id = 0;
  while (id < NUM_PROCESSES && state->processes[id].state != Free) {
    id += 1;
  }

  /* One cannot create a child process with a higher priority than its own */
  if (id == NUM_PROCESSES || child_prio > state->processes[state->curr_pid].prio) {
    state->processes[state->curr_pid].context.eax = 0;
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
  state->processes[state->curr_pid].context.eax = 1;
  state->processes[state->curr_pid].context.ebx = id;
}

/**
 * @name resolve_exit_wait - Resolves an exit or wait syscall
 * @param parent           - The parent process, in waiting mode
 * @param child            - The child process, in zombie mode
 * @return void
 */
void resolve_exit_wait(pid parent, pid child)
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
  parent_proc->context.ecx = state->processes[child].context.ebx;  /* Return value */
}

void syscall_exit()
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
    resolve_exit_wait(parent_id, id);
  }
}

void syscall_wait()
{
  state->processes[state->curr_pid].state = Waiting;
  pid parent_id = state->curr_pid;
  bool has_children = FALSE;

  for (pid id = 0; id < NUM_PROCESSES; id++) {
    process_t proc = state->processes[id];
    if (proc.parent_id == parent_id) {
      has_children = TRUE;
      if (proc.state== Zombie) {
        resolve_exit_wait(parent_id, id);
        return;
      }
    }
  }

  if (!has_children) {
    /* The process has no children, the call terminates instantly */
    state->processes[state->curr_pid].context.eax = 0;
    state->processes[state->curr_pid].state = Runnable;
  }
}


void syscall_invalid()
{
  throw("Invalid syscall!");
}


void syscall_handler(regs_t *regs)
{
  process_t *proc = &state->processes[state->curr_pid];

  /* Save context */
  proc->context = *regs;

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
  if (proc->state != Runnable) {
      select_new_process();
    }

  transfer_control(&state->processes[state->curr_pid]);  /* Beware, not proc! */
}
