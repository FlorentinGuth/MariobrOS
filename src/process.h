#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "paging.h"


/* The id of a process */
typedef unsigned int pid;

/* The priority of a process */
typedef int priority;           /* 0 indicates a weak priority, MAX_PRIORITY the strongest one */
#define MAX_PRIORITY     15     /* Priorities range from 0 to 15 */


/* The state of the process */
typedef enum process_state {
  Free = 0, /* The process does not refer to actual code */
  Waiting,  /* The process is waiting for onf of his child processes to die */
  Runnable, /* The process does not wait for any result and can be executed */
  Zombie,   /* The process returned, and his parent process has not yet called wait */
} process_state_t;


/* The context of the process: all the variables he need */
typedef struct context {
  /* Registers */
  regs_t *regs;  /* The registers of the process */
  u_int32 esp;   /* The esp of the handler call, to restore the stack after switching */

  /* Malloc state */
  void *first_free_block;
  void *unallocated_mem;

  /* Paging state */
  page_directory_t *page_dir;
} context_t;

context_t kernel_context;


/* A process */
typedef struct process {
  process_state_t state;

  pid      parent_id;
  priority prio;

  context_t context;
} process_t;


/**
 * @name new_process      - Returns a new process with a clean paging and malloc state
 *                          What remains to initialize is the kernel esp, regs->esp and regs->eip
 * @param parent_id       - Identifier of the parent process
 * @param prio            - Priority of the process
 * @param create_page_dir - Whether to create a fresh new page directory
 * @return process_t
 */
process_t new_process(pid parent_id, priority prio, bool create_page_dir);


#endif
