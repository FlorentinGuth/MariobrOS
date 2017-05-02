#ifndef QUEUE_H
#define QUEUE_H

#include "types.h"


/**
 * The type of a queue structure, which is a circular doubly-linked list.
 */
typedef struct queue* queue_t;
struct queue {
  u_int32 value;
  queue_t prev;
  queue_t next;
};

/**
 * @name empty_queue - Allocates a new, empty queue
 * @return An empty queue, which will need to be eventually freed
 */
queue_t *empty_queue();
/**
 * @name is_empty_queue - Tests whether the given queue is empty
 * @param q             - A pointer to the queue (pointer must be non-null)
 * @return TRUE if the queue is empty, FALSE otherwise
 */
bool is_empty_queue(queue_t *q);

/**
 * @name enqueue - Inserts an element at the start of the queue
 * @param q      - A pointer to the queue
 * @param x      - The element to insert
 * @return void
 */
void enqueue(queue_t *q, u_int32 x);
/**
 * @name dequeue - Deletes the element at the end of the queue
 * @param q      - A non-empty queue
 * @return The element at the end of the queue
 */
u_int32 dequeue(queue_t *q);

#endif
