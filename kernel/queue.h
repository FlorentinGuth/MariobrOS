#ifndef QUEUE_H
#define QUEUE_H

struct Queue {
    int value;
    struct Queue* prev;
    struct Queue* next;
};
typedef struct Queue* queue;

queue* empty_queue();
int    is_empty_queue(queue*);

void enqueue(queue*, int);
int  dequeue(queue*);

#endif