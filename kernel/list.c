#include "list.h"
#include <stdlib.h>

void append(list* l, int v)
{
    struct List* u = malloc(sizeof(struct List));
    u->head = v;
    u->tail = *l;
    *l = u;
}

void remove_elt(list* l, int v)
{
    if (*l)
    {
        if ((*l)->head == v)
        {
            struct List* tail = (*l)->tail;
            free(*l);
            *l = (*l)->tail;
        }
        else
        {
            struct List* curr = *l;
            while (curr->tail && curr->tail->head != v)
            {
                curr = curr->tail;
            }
            if (curr->tail)
            {
                struct List* tail = curr->tail->tail;
                free(curr->tail);
                curr->tail = tail;
            }
        }
    }
}


int max(int a, int b) { return a > b ? a : b; }
int max_list(list* l) // Assumes *l != NULL
{
    int max_elt = (*l)->head;
    struct List* curr = (*l)->tail;
    while (curr)
    {
        max_elt = max(max_elt, curr->head);
        curr = curr->tail;
    }
    return max_elt;
}

int extract_max(list* l)
{
    int max_elt = max_list(l);
    remove_elt(l, max_elt);
    return max_elt;
}