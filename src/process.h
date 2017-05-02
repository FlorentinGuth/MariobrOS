#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "paging.h"


/* The id of a process */
typedef unsigned int pid;

typedef int priority;           /* 0 indicates a weak priority, MAX_PRIORITY the strongest one */
#define MAX_PRIORITY     15     /* Priorities range from 0 to 15 */

typedef enum process_state {
  Free,     /* The process does not refer to actual code */
  Waiting,  /* The process is waiting for onf of his child processes to die */
  Runnable, /* The process does not wait for any result and can be executed */
  Zombie,   /* The process returned, and his parent process has not yet called wait */
} process_state_t;

typedef struct process {
  process_state_t state;

  pid      parent_id;
  priority prio;

  regs_t context;
  page_directory_t *page_dir;
} process_t;


/**
 * @name transfer_control - Restores the process context and runs it
 * @param proc            - The process to transfer control to
 * @return void
 */
void transfer_control(process_t *proc);


#endif
