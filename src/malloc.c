#include "malloc.h"
#include "paging.h"
#include "memory.h"
#include "logging.h"
#include "types.h"
#include "math.h"
#include "utils.h"

/* Reminder: beware to pointer arithmetic.
 * Adding 1 means getting access to the next element, i.e. adds sizeof(type)...
 * To deal with bytes, use void* (or functions like get_next_block...).
 */


/* Unique end header for all blocks */
typedef struct end_header
{
  size_t size : 31;  /* Shifted right one bit */
  bool used   :  1;  /* 0 for free, 1 for used, strongest bit because of little-endianness */
} __attribute__((packed)) end_header_t;

/* Used block have a first header identical to the end one */
typedef end_header_t header_used_t;

/* Free blocks also have the address of the adjacent free blocks in the doubly linked list */
typedef struct header_free header_free_t;
struct header_free
{
  size_t size : 31;
  bool used   :  1;
  header_free_t *prev;        /* Pointer to the previous free block in the doubly chained list */
  header_free_t *next;        /* Pointer to the next free block in the doubly chained list */
} __attribute__((packed));

/* Structure of a free block (size always comprises headers):
 * - header_free_t
 * - free memory (unspecified)
 * - end_header_t
 */

/* Structure of an used block (size always comprises headers and padding, big enough to be freed):
 * - header_used_t
 * - maybe some padding bytes, set to 0 (always different than the header used since its last bit is 1)
 * - used memory (domain of the user)
 * - end_header_t
 */


size_t get_size(void *block)
{
  return 2*(((end_header_t *)block)->size);
}

bool get_used(void *block)
{
  return ((end_header_t *)block)->used;
}


header_used_t *get_first_header(void *end_header)
{
  return end_header + sizeof(end_header_t) - get_size(end_header);
}

end_header_t *get_end_header(void *first_header)
{
  return first_header + get_size(first_header) - sizeof(end_header_t);
}

void *get_prev_block(void *block)
{
  if (block - sizeof(end_header_t) < (void *)END_OF_KERNEL_HEAP)
    return 0;  /* In case we pass the boundary */
  return block - get_size(block - sizeof(end_header_t));
}

void *get_next_block(void *block)
{
  void *next = block + get_size(block);
  if (next < unallocated_mem)
    return next;
  return 0;  /* In case we pass the boundary */
}


void write_block(void *block)
{
  if (!block) {
    writef("Nonexistent block\n");
    return;
  }

  if (!get_used(block)) {
    header_free_t *a = (header_free_t *)block;
    writef("Free block at %x, size %x, prev %x, next %x\n", a, get_size(a), a->prev, a->next);
  } else {
    header_used_t *a = (header_used_t *)block;
    writef("Used block at %x, size %x\n", a, get_size(a));
  }
}

void log_block(void *block)
{
  if (!block) {
    kloug(100, "Nonexistent block\n");
    return;
  }

  if (!get_used(block)) {
    header_free_t *a = (header_free_t *)block;
    kloug(100, "Free block at %x, size %x, prev %x, next %x\n", a, get_size(a), a->prev, a->next);
  } else {
    header_used_t *a = (header_used_t *)block;
    kloug(100, "Used block at %x, size %x\n", a, get_size(a));
  }
}

void write_memory()
{
  writef("First free block at %x\n", first_free_block);
  void* block = (void *)END_OF_KERNEL_HEAP;
  while (block) {
    write_block(block);
    if (get_size(block) == 0) {
      writef("Block with size 0!\n");
      return;
    }
    block = get_next_block(block);
  }
}

void log_memory()
{
  kloug(100, "First free block at %x\n", first_free_block);
  void* block = (void *)END_OF_KERNEL_HEAP;
  while (block) {
    log_block(block);
    block = get_next_block(block);
  }
}


void set_size(void *block, size_t size)
{
  ((end_header_t *)block)->size = size / 2;  /* Shift one bit right (unsigned) */
}

void set_block(void* block, size_t size, bool used)
{
  kloug(100, "Setting block at %x with size %x, used %u\n", block, size, used);
  set_size(block, size);
  ((header_used_t *)block)->used = used;

  void *end_header = get_end_header(block);
  *(u_int32 *)end_header = *(u_int32 *)block;  /* Recopying header */
  /* kloug(100, "Verification: %x %x %u\n", end_header, get_size(block), get_used(block)); */
}


void insert_after(header_free_t *to_insert, header_free_t *block)
{
  kloug(100, "Inserting block at %x after block at %x\n", to_insert, block);
  if (block) {
    /* a->b becomes a->c->b */
    header_free_t *a = block;
    header_free_t *b = a->next;
    header_free_t *c = to_insert;

    a->next = c;
    c->prev = a;
    c->next = b;
    if (b) {
      b->prev = c;
    }
  } else {
    kloug(100, "Inserting at the start of the queue (ffb at %x)\n", first_free_block);
    log_block(get_next_block(to_insert));
    to_insert->next = first_free_block;
    to_insert->prev = 0;
    if (first_free_block)
      ((header_free_t *)first_free_block)->prev = to_insert;
    first_free_block = to_insert;
  }
}

void remove(header_free_t *block)
{
  kloug(100, "Removing block at %x from free list\n", block);
  log_memory();
  /* a->c->b becomes a->b */
  header_free_t *c = block;
  header_free_t *a = c->prev;
  header_free_t *b = c->next;

  if (a)
    a->next = b;
  else
    first_free_block = b;
  if (b)
    b->prev = a;
}


void merge_with_next(header_free_t *block)
{
  kloug(100, "Merging block at %x with the next one\n", block);
  header_free_t *a = block, *b = get_next_block(block);

  log_block(a); log_block(b);

  if (b && !b->used) {
    kloug(100, "Actively merging with block at %x\n", b);
    /* c->a->d and e->b->f becomes c->a+b->d and e->f */
    /* It is possible that c = f or e = d, but it does not change anything */
    header_free_t *e = b->prev, *f = b->next;
    if (e)
      e->next = f;
    else
      first_free_block = f;
    if (f)
      f->prev = e;

    set_size(a, get_size(a) + get_size(b));  /* Actual merging */
  }
}

header_free_t *merge(header_free_t *block)
{
  merge_with_next(block);
  header_free_t *prev = get_prev_block(block);
  if (prev && !prev->used) {
    merge_with_next(prev);
    return prev;
  } else {
    return block;
  }
}


void malloc_install()
{
  /* Allocates a page */
  alloc_frame(get_page(END_OF_KERNEL_HEAP, TRUE, current_directory), FALSE, TRUE);
  unallocated_mem = (void *)(END_OF_KERNEL_HEAP + 0x1000);

  first_free_block = (header_free_t *)END_OF_KERNEL_HEAP;
  set_block(first_free_block, 0x00001000, FALSE);  /* Setting size of 4KB: the whole page */
  kloug(100, "Malloc installed\n");
}


/**
 * @name alloc_pages - Allocates pages to have a free block big enough to fit the given size
 * @param size -  size of the block
 * @return The new free block
 */
header_free_t *alloc_pages(size_t size)
{
  kloug(100, "Let's try to allocate a page\n");

  void *last_block = get_prev_block(unallocated_mem);
  size_t last_size = get_size(last_block);
  bool free = !get_used(last_block);
  /* writef("Last physical block at %x, size %x, free: %u\n", last_block, last_size, free); */

  int nb_pages;
  if (free) {
    nb_pages = ceil_ratio(size - last_size, 0x1000);
  } else {
    nb_pages = ceil_ratio(size, 0x1000);
  }
  header_free_t *block = unallocated_mem;

  kloug(100, "Allocating %u page(s)\n", nb_pages);
  for (int i = 0; i < nb_pages; i++) {
    alloc_frame(get_page((u_int32)unallocated_mem, TRUE, current_directory), FALSE, TRUE);
    unallocated_mem += 0x1000;
  }

  set_block(block, nb_pages * 0x1000, FALSE);
  insert_after(block, 0);  /* Inserts at the start of the list */
  return merge(block);     /* Merge with previous block if needed */
}

void *mem_alloc_aligned(size_t size, unsigned int alignment)
{
  kloug(100, "Allocating a block of size %x\n", size);
  log_memory();
  /* We need:
   * - a block big enough to contain the asked size, including the header_used and end_header
   * - the returned address (address of block + sizeof(header_used_t)) should be aligned
   * - the address of the new block (is we create one) should be 2-bytes-aligned
   */

  /* Tentative WIP version */
  /* header_free_t *block = first_free_block; */
  /* while (block && */
  /*        ceil_multiple((u_int32)block + sizeof(header_used_t), alignment) + size + sizeof(end_header_t) */
  /*        > (u_int32)block + get_size(block)) { */
  /*   block = block->next; */
  /* } */
  /* if (!block) { */
  /*   /\* Overestimating the size to be sure to have enough *\/ */
  /*   size_t needed = sizeof(header_used_t) + ceil_multiple(size, 2) + alignment - 1 + sizeof(end_header_t); */
  /*   block = alloc_pages(needed); */
  /* } */

  /* /\* We have potentially 3 blocks: */
  /*  * - a small portion (depending on alignment) before the user block */
  /*  * - the user block */
  /*  * - what's left of the initial block */
  /*  *\/ */
  /* /\* What we will return to the user, which must be aligned *\/ */
  /* u_int32 addr_of_free_mem = ceil_multiple((u_int32)block + sizeof(header_used_t), alignment); */
  /* /\* The address of the start of the block, which must be 2-bytes-aligned *\/ */
  /* u_int32 addr_of_block = floor_multiple(addr_of_free_mem - sizeof(header_used_t), 2);  /\* >= block *\/ */
  /* /\* The address of the block right after the one we will create, also 2-bytes-aligned *\/ */
  /* u_int32 addr_of_next_block = ceil_multiple(addr_of_block + size + sizeof(end_header_t), 2); */

  /* Getting lazy here, so will just waste space */
  /* Calculate the size of the final block, which must respect the following conditions:
   * - be big enough to allow conversion into a free block later
   * - be big enough to insert padding to return an aligned address
   * - be a multiple of two
   */
  size = ceil_multiple(max(sizeof(header_used_t)
                           + alignment - 1
                           + size
                           + sizeof(end_header_t),
                           sizeof(header_free_t)
                           + sizeof(end_header_t)),
                       2);
  header_free_t *block = first_free_block;
  while (block && get_size(block) < size) {
    block = block->next;
  }
  if (!block) {
    block = alloc_pages(size);
  }

  remove(block);
  size_t size_block = get_size(block);
  kloug(100, "Block of size %x inside block of size %x\n", size, size_block);

  if (size_block - size >= sizeof(header_free_t) + sizeof(end_header_t)) {
    /* We create a new block after the one we give */
    set_block(block, size, TRUE);
    void *next = get_next_block(block);
    set_block(next, size_block - size, FALSE);
    insert_after(next, 0);
    merge_with_next(next);
  } else {
    set_block(block, size_block, TRUE);
  }

  /* Padding with zeroes and returning free memory address */
  u_int32 start = (u_int32)block;
  u_int32 after_header = start + sizeof(header_used_t);
  u_int32 after_padding = ceil_multiple(after_header, alignment);
  mem_set((void *)after_header, 0, after_padding - after_header);
  return (void *)after_padding;
}

void *mem_alloc(size_t size)
{
  return mem_alloc_aligned(size, 1);
}


void mem_free(void *ptr)
{
  /* TODO: free pages when not used anymore? */
  /* Passing padding bytes to find back the header */
  /* Beware! Little-endian, i.e. a 4-bytes int 0xABCDEF00 is stored as 00 EF CD AB... */
  u_int8 *maybe_header = (u_int8 *)ptr - 1;
  while (*maybe_header == 0) {
    maybe_header--;
  }
  maybe_header++;
  header_free_t *block = (header_free_t *)(maybe_header - sizeof(header_used_t));

  kloug(100, "Freeing block at %x (supplied %x, maybe %x)\n", block, ptr, maybe_header);
  log_memory();

  block->used = FALSE;
  insert_after(block, 0);
  merge(block);
  /* write_block(block); */
}
