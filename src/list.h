#ifndef LIST_H
#define LIST_H

#include "types.h"


typedef struct list* list_t;
struct list {
    u_int32 head;
    list_t tail;
};

void append(list_t *l, u_int32 x);

void remove_elt(list_t *l, u_int32 x); // First occurrence only


u_int32 max_list(list_t *l);

u_int32 extract_max(list_t *l);


void reverse(list_t *l);

/**
 * @name find    - Finds the first element in l satisfying the given predicate
 * @param l      - The list
 * @param remove - If true, removes the found element
 * @return The element satisfying p, or 0 if none was found
 */
u_int32 find(list_t *l, bool (*predicate)(u_int32 x), bool remove);

#endif
