#ifndef SYSCALL_H
#define SYSCALL_H

#include "scheduler.h"


#define SYSCALL_ISR 20

typedef enum syscall {
  Exit,    /* The process is finished and returns a value */
  Fork,    /* Creates a new child process, with the same context at first */
  Wait,    /* Waits for a child to return a value */
} syscall_t;

/**
 * @name syscall_fork - Creates a new process with a new, copied context
 * This syscall has one param, in ebx: the priority to give to the child process,
 * which must be less than or equal to the priority of the current process.
 * If there's no free process, or if the child priority is higher than the priority
 * of the current process, the call terminates and places 0 in eax.
 * Otherwise, the parent process has 1 in eax and the pid of the child process in ebx,
 * while the child process has 2 in eax and the pid of the parent process in ebx.
 * @return void
 */
void syscall_fork();
/**
 * @name syscall_exit  - Terminates the process
 * This syscall has one param, in ebx: the return value of the exiting process.
 * The child of the process have their parent replaced by the init process, while
 * the parent of the process will be given (once he calls wait) the return value.
 * @return void
 */
void syscall_exit();
/**
 * @name syscall_wait - Waits for a child process to call exit
 * If the process has no child processes, 0 will be placed in eax.
 * Otherwise, 1 is placed in eax, the pid of the exiting child process in ebx
 * and its return value in ecx.
 * @return void
 */
void syscall_wait();

void syscall_invalid();
#endif
