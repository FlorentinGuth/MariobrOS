#ifndef PAGING_ASM_H
#define PAGING_ASM_H


/** enable_paging:
 *  Enables paging, with the page directory table being in the variable `pdt`.
 */
void enable_paging();

/** paging_set_4mb:
 *  Sets the page size to 4MB instead of 4KB.
 *  DO NOT USE. This is here just to remember how one does it if I ever need it.
 */
void paging_set_4mb();

#endif
