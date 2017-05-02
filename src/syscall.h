#ifndef SYSCALL_H
#define SYSCALL_H

#include "scheduler.h"


typedef enum syscall {
  Exit,    /* The process is finished and returns a value */

  Fork,    /* Creates a new child process, with the same context at first */
  Wait,    /* Waits for a child to return a value */

  Invalid, /* Invalid syscall value */
} syscall_t;

/**
 * @name syscall_fork - Creates a new process with a new, copied context
 * If there's no free process, or if the child priority is higher than the priority
 * of the current process, the call terminates and places 0 in eax.
 * Otherwise, the parent process has 1 in eax and the pid of the child process in ebx,
 * while the child process has 2 in eax and the pid of the parent process in ebx.
 * @param state       - The scheduler state
 * @param child_prio  - The priority to give to the child process, which must be
 *                      less than or equal to the priority of the current process
 * @return void
 */
void syscall_fork(scheduler_state_t *state, priority child_prio);
/**
 * @name syscall_exit  - Terminates the process
 * The child of the process have their parent replaced by the init process, while
 * the parent of the process will be given (once he calls wait) the return value.
 * @param state        - The scheduler state
 * @param return_value - The return value of the exiting process
 * @return void
 */
void syscall_exit(scheduler_state_t *state, u_int32 return_value);
/**
 * @name syscall_wait - Waits for a child process to call exit
 * If the process has no child processes, 0 will be placed in eax.
 * Otherwise, 1 is placed in eax, the pid of the exiting child process in ebx
 * and its return value in ecx.
 * @param state       - The scheduler state
 * @return void
 */
void syscall_wait(scheduler_state_t *state);


#endif
