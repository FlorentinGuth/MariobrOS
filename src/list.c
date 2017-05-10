#include "list.h"
#include "malloc.h"
#include "utils.h"


list_t *empty_list()
{
  list_t *l = mem_alloc(sizeof(list_t));
  *l = NULL;
  return l;
}

bool is_empty_list(list_t *l)
{
  return *l == NULL;
}


void push(list_t *l, u_int32 x)
{
  list_t u = mem_alloc(sizeof(struct list));
  u->head = x;
  u->tail = *l;
  *l = u;
}

u_int32 pop(list_t *l)
{
  u_int32 x = (*l)->head;
  list_t tail = (*l)->tail;

  mem_free(*l);
  *l = tail;

  return x;
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


u_int32 max_list(list_t *l)
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
  list_t rev = NULL;
  list_t curr = *l;
  while (curr) {
    push(&rev, curr->head);
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



void delete_list(list_t *l, bool free_elts)
{
  while (!is_empty_list(l)) {
    u_int32 x = pop(l);
    if (free_elts) {
      mem_free((void *)x);
    }
  }

  mem_free(l);
}
