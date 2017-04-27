#include "list.h"
#include "malloc.h"
#include "utils.h"


void append(list_t *l, u_int32 x)
{
  list_t u = mem_alloc(sizeof(struct list));
  u->head = x;
  u->tail = *l;
  *l = u;
}

void remove_elt(list_t *l, u_int32 x)
{
  if (*l) {
    if ((*l)->head == x) {
      list_t tail = (*l)->tail;
      mem_free(*l);
      *l = tail;
    } else {
      list_t curr = *l;
      while (curr->tail && curr->tail->head != x) {
        curr = curr->tail;
      }
      if (curr->tail) {
        list_t tail = curr->tail->tail;
        mem_free(curr->tail);
        curr->tail = tail;
      }
    }
  }
}


u_int32 max_list(list_t *l) // Assumes *l != NULL, i.e. non-empty list
{
    u_int32 max_elt = (*l)->head;
    list_t curr = (*l)->tail;
    while (curr)
    {
        max_elt = max(max_elt, curr->head);
        curr = curr->tail;
    }
    return max_elt;
}

u_int32 extract_max(list_t *l)
{
    u_int32 max_elt = max_list(l);
    remove_elt(l, max_elt);
    return max_elt;
}


void reverse(list_t *l)
{
  list_t rev = 0;
  list_t curr = *l;
  while (curr) {
    append(&rev, curr->head);
    curr = curr->tail;
  }
  *l = rev;
}

u_int32 find(list_t *l, bool (*p)(u_int32), bool remove)
{
  list_t curr = *l;
  while (curr) {
    if (p(curr->head)) {
      if (remove) {
        remove_elt(l, curr->head);
      }
      return curr->head;
    }
    curr = curr->tail;
  }
  return 0;
}
