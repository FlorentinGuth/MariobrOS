#include "queue.h"
#include <stdlib.h>

queue* empty_queue()
{
    queue* q = malloc(sizeof(queue));
    *q = NULL;
    return q;
}

int is_empty_queue(queue* q)
{
    return *q == NULL;
}


void enqueue(queue* q, int v)
{
    struct Queue* elt = malloc(sizeof(struct Queue));
    elt->value = v;

    if (*q)
    {
        elt->next = *q;
        elt->prev = (*q)->prev;
        (*q)->prev->next = elt;
        (*q)->prev = elt;
    }
    else
    {
        elt->next = elt;
        elt->prev = elt;
        *q = elt;
    }
}

int dequeue(queue* q)
{
    struct Queue* elt = *q; // elt != 0
    int v = elt->value;

    struct Queue* next = elt->next;
    struct Queue* prev = elt->prev;

    next->prev = prev;
    prev->next = next;
    free(elt);

    if (next == elt)
    {
        // There was only one element left
        *q = NULL;
    }
    else
    {
        *q = next;
    }

    return v;
}