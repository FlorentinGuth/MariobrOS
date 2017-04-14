#include "malloc.h"
#include "logging.h"


/** Memory layout:
 *  The memory is split up into blocks. Each block has a header, which contains:
 *   - Its size (4 byte, so up to 4GB)
 *   - The address of the next free block (4 byte, 4-byte-alignment implies that the last two bits are 0)
 *   - One of those bit is used to tell if the block is used (1) or free (0)
 *   - The last bit is currently unused.
 *   - The address of the previous free block
 *  Each block is laid out as follows:
 *   - Its header (12 bytes)
 *   - Used or free memory for the user
 *   - Optionally, some filler bytes so the total size is a multiple of 4
 *   - The replication of its header (12 bytes)
 */
/** TODO: new architecture:
 *  The end header contains only the size, with the last bit indicating whether the block is free or not.
 *  The first header is the same as the end in case of used blocks, and the same as previously for free blocks
 *  2-byte-alignment is sufficient since we only need one bit */

#define WORD_SIZE   4
#define HEADER_SIZE 3 * WORD_SIZE

void *first_free_block;

size_t get_size(void *block)
{
  return *(size_t *)block;
}

bool get_used(void *block)
{
  return *(u_int32 *)(block + WORD_SIZE) & 0x00000001;
}

void *get_next_free_block(void *block)
{
  return (void *)(*(u_int32 *)(block + WORD_SIZE) & 0xFFFFFFFC);
}

void *get_previous_free_block(void *block)
{
  return (void *)(*(u_int32 *)(block + 2*WORD_SIZE) & 0xFFFFFFFC);
}

void *get_previous_block(void *block)
{
  void *header = block - HEADER_SIZE;  /* Header of the previous block in memory */
  return block - get_size(header);
}

void *get_next_block(void *block)
{
  return block + get_size(block);
}


void set_size(void *block, size_t size)
{
  *(size_t *)block = size;
}

void set_next_free_block(void *block, void *next)
{
  *(u_int32 *)(block + WORD_SIZE) = ((u_int32)next & 0xFFFFFFFC) | (u_int32)get_used(block);
}

void set_previous_free_block(void *block, void *prev)
{
  *(u_int32 *)(block + 2*WORD_SIZE) = ((u_int32)prev & 0xFFFFFFFC);
}

void set_used(void *block, bool used)
{
  *(u_int32 *)(block + WORD_SIZE) = (u_int32)get_next_free_block(block) | used;
}


void set_block(void* ptr, size_t size, void *next, void *prev, bool used)
{
  set_size(ptr, size);
  set_next_free_block(ptr, next); /* Could do these two together, but I'm too lazy */
  set_used(ptr, used);
  set_previous_free_block(ptr, prev);

  void *header_end = ptr + size - HEADER_SIZE;
  set_size(header_end, size);
  set_next_free_block(header_end, next);
  set_used(header_end, used);
  set_previous_free_block(header_end, prev);
}


void malloc_install()
{
  first_free_block = (void *)END_OF_KERNEL_HEAP;
  set_block(first_free_block, 0x00800000, 0, 0, FALSE);  /* Setting size of 8MB, total hack */
  log_string("Malloc installed\n", Info);
}

void *mem_alloc(size_t size)
{
  log_string("Allocating a block\n", Info);
  size = size + 2*HEADER_SIZE;  /* Account for the two headers */
  if (size % 4) {
    size = (size & 0xFFFFFFFC) + 4;
  }

  void *block = first_free_block;

  while (block && (get_used(block) || get_size(block) < size)) {
    block = get_next_free_block(block);
  }

  if (!block) {
    log_string("No free memory\n", Debug);
    return 0;  /* No free block big enough */
    /* TODO: allocate a new page */
  }

  size_t size_block = get_size(block);
  if (size_block - size >= WORD_SIZE + 2*HEADER_SIZE) {
    log_string("There is enough place for another block\n", Debug);
    void *previous = get_previous_free_block(block);
    void *next     = get_next_free_block(block);

    set_block(block,        size,              0,    0,        TRUE);
    set_block(block + size, size_block - size, next, previous, FALSE);

    if (next) {
      log_string("There is a next free block\n", Debug);
      set_previous_free_block(next, block + size);
    }
    if (previous) {
      log_string("There is a previous free block\n", Debug);
      set_next_free_block(previous, block + size);
    } else {
      first_free_block = block + size;
    }
  } else {
    /* There is not enough place for another block */
    void *previous = get_previous_free_block(block);
    void *next     = get_next_free_block(block);

    set_block(block, size_block, 0, 0, TRUE);

    if (next) {
      set_previous_free_block(next, previous);
    }
    if (previous) {
      set_next_free_block(previous, next);
    } else {
      first_free_block = next;
    }
  }

  return block + HEADER_SIZE;  /* Pointer to the free memory, after the header */
}

void mem_free(void *block)
{
  /* We maintain the invariant that the doubly-linked list of free blocks is sorted by address */
  /* In case this changes:
     If A and B are contiguous free blocks, we update the list from C->A->D & E->B->F to
     C->A+B->D & E->F */
  void *previous;
  if ((u_int32)block > 0x00800000 + HEADER_SIZE) { /* TODO: merge this into get_previous_block */
    previous = get_previous_block(block);
  } else {
    previous = 0;
  }
  void *next = get_next_block(block);
  if ((u_int32)next >= 0x01000000) {
    next = 0;
  }

  void *new_block;
  void *previous_free;
  void *next_free;
  size_t size;
  /* TODO: refactor this into non-nested conditions */
  if (!previous || get_used(previous)) {
    new_block = block;
    previous_free = get_previous_free_block(block);
    if (!next || get_used(next)) {
      /* Neither blocks are free, simple case */
      size = get_size(block);
      next_free = get_next_free_block(block);
    } else {
      /* Only the next block is free */
      size = get_size(block) + get_size(next);
      next_free = get_next_free_block(next);
    }
  } else {
    new_block = previous;
    previous_free = get_previous_free_block(previous);
    if (!next || get_used(next)) {
      /* Only the previous block is free */
      size = get_size(previous) + get_size(block);
      next_free = get_next_free_block(block);
    } else {
      /* Both blocks are free */
      size = get_size(previous) + get_size(block) + get_size(next);
      next_free = get_next_free_block(next);
    }
  }

  set_block(new_block, size, next_free, previous_free, FALSE);
  if (!previous_free) {
    first_free_block = new_block;
  }
}
