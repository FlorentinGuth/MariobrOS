#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "types.h"
#include "queue.h"
#include "process.h"


#define NUM_PROCESSES   128  /* Maximum number of concurrent processes */
#define MAX_TIME_SLICES   5  /* Number of timer events before switching processes */

typedef struct scheduler_state {
  pid      curr_pid;
  int      time_slices_left;

  process_t processes[NUM_PROCESSES];
  queue_t  *runqueues[MAX_PRIORITY + 1];  /* Set of process ids ordered by priority */
} scheduler_state_t;


/**
 * @name scheduler_install - Initializes and return a new scheduler
 * @return scheduler_state_t*
 */
scheduler_state_t *scheduler_install();

/**
 * @name send_timer_event - Sends a timer event to the current process, switching it if needed
 * @param state           - The scheduler state
 * @return void
 */
void send_timer_event(scheduler_state_t *state);


#endif /* SCHEDULER_H */
