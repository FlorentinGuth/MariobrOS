#include "queue.h"
#include "types.h"
#include "malloc.h"


queue_t *empty_queue()
{
  queue_t *q = mem_alloc(sizeof(queue_t));
  *q = NULL;
  return q;
}

bool is_empty_queue(queue_t *q)
{
  return *q == NULL;
}


void enqueue(queue_t *q, u_int32 v)
{
  queue_t elt = mem_alloc(sizeof(struct queue));
  elt->value = v;

  if (*q) {
    elt->next = *q;
    elt->prev = (*q)->prev;

    (*q)->prev->next = elt;
    (*q)->prev = elt;
  } else {
    elt->next = elt;
    elt->prev = elt;

    *q = elt;
  }
}

u_int32 dequeue_t(queue_t *q)
{
  queue_t elt = *q;        /* We assume *q != 0 */
  u_int32 v = elt->value;

  queue_t next = elt->next;
  queue_t prev = elt->prev;

  next->prev = prev;
  prev->next = next;
  mem_free(elt);

  if (next == elt) {
    /* There was only one element left */
    *q = NULL;
  } else {
    *q = next;
  }

  return v;
}
