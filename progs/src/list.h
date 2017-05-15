#ifndef LIST_H
#define LIST_H

#include "lib.h"

typedef struct list* list_t;
struct list {
    u_int32 head;
    list_t tail;
};

/**
 * @name empty_list - Allocates a new, empty list
 * @return          - A pointer to an empty list, which will need to be eventually freed
 */
list_t *empty_list();
/**
 * @name is_empty_list - Tests whether the given list is empty
 * @param l            - A non-null pointer to the list
 * @return             - TRUE if the list is empty, FALSE otherwise
 */
bool is_empty_list(list_t *l);

/**
 * @name push - Inserts an element at the start of the list
 * @param l   - The list
 * @param x   - The element to insert
 * @return void
 */
void push(list_t *l, u_int32 x);
/**
 * @name pop - Removes the element at the start of the list, and returns it
 * @param l  - A non-empty list
 * @return   - The element at the start of the list
 */
u_int32 pop(list_t* l);

/**
 * @name remove_elt - Removes the first occurrence of an element inside a list
 * @param l         - The list
 * @param x         - The element to remove
 * @return void
 */
void remove_elt(list_t *l, u_int32 x);

/**
 * @name max_list - Returns the maximum of the element contained in the list
 * @param l       - The list, which must be non-empty
 * @return u_int32
 */
u_int32 max_list(list_t *l);
/**
 * @name extract_max - Returns the maximum element of the list, and removes its first occurrence
 * @param l          - A non-empty list
 * @return u_int32
 */
u_int32 extract_max(list_t *l);

/**
 * @name reverse - Reverses the list
 * @param l      - The list
 * @return void
 */
void reverse(list_t *l);

/**
 * @name find       - Finds the first element in l satisfying the given predicate
 * @param l         - The list
 * @param predicate - The predicate
 * @param remove    - If true, removes the found element
 * @return          - The element satisfying p, or 0 if none was found
 */
u_int32 find(list_t *l, bool (*predicate)(u_int32 x), bool remove);

/**
 * @name delete_list - Frees a whole list
 * @param l          - The list to free
 * @param free_elts  - Whether to free its elements (which are interpreted as pointers)
 * @return void
 */
void delete_list(list_t *l, bool free_elts);

#endif
