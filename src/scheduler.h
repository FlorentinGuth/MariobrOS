#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "types.h"
#include "string.h"
#include "queue.h"
#include "process.h"


#define NUM_PROCESSES   128  /* Maximum number of concurrent processes */
#define SWITCH_FREQ    1000  /* Frequence (in Hz) of the switching */

typedef struct scheduler_state {
  pid      curr_pid;

  process_t processes[NUM_PROCESSES];
  queue_t  *runqueues[MAX_PRIORITY + 1];  /* Set of process ids ordered by priority */
} scheduler_state_t;


/**
 * @name scheduler_install - Initializes a new scheduler
 * @return void
 */
void scheduler_install();

/**
 * @name select_new_process - Searches the runqueues for a runnable process with highest priority
 * @return void
 */
void select_new_process();

/**
 * @name run_program - Runs the given program
 * @param name       - The name of the program, /progs/name.elf must exist
 * @return void
 */
void run_program(string name);


#endif /* SCHEDULER_H */
