#ifndef LIST_H
#define LIST_H

struct List {
    int head;
    struct List* tail;
};
typedef struct List* list;

void append(list*, int);
void remove_elt(list*, int); // First occurence only
int max_list(list*);
int extract_max(list*);

#endif