#include "malloc.h"
#include "paging.h"
#include "logging.h"
#include "math.h"


/** Memory layout:
 *  The memory is split up into blocks. Each block has a header, which contains:
 *   - Its size (4 bytes, so up to 4GB)
 *   - The address of the next free block (4 bytes)
 *       -> 4-byte-alignment implies that the last two bits are 0
 *       -> The last bit is used to tell if the block is used (1) or free (0)
 *       -> The remaining bit is currently unused.
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
#define HEADER_SIZE 3*WORD_SIZE

void *first_free_block;
void *unallocated_mem;

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
  return block - get_size(block - HEADER_SIZE);
}

void *get_next_block(void *block)
{
  return block + get_size(block);
}

/* void *get_physical_previous_free_block(void *block) */
/* { */
/*   while(get_used(block) && (u_int32)block >= END_OF_KERNEL_HEAP) { */
/*     block -= get_size(block - HEADER_SIZE); */
/*   } */
/*   if((u_int32)block < 0x00800000) */
/*     return 0; */
/*   return block; */
/* } */

/* void *get_physical_next_free_block(void *block) */
/* { */
/*   while(get_used(block) && (u_int32)block < 0x00800000 + END_OF_KERNEL_HEAP) { */
/*     block += get_size(block + get_size(block)); */
/*   } */
/*   if((u_int32)block >= 0x00800000 + END_OF_KERNEL_HEAP) */
/*     return 0; */
/*   return block; */
/* } */

void set_size(void *block, size_t size)
{
  *(size_t *)block = size;
}

void set_next_free_block(void *block, void *next)
{
  *(u_int32 *)(block + WORD_SIZE) = \
    ((u_int32)next & 0xFFFFFFFC) | (u_int32)get_used(block);
}

void set_previous_free_block(void *block, void *prev)
{
  *(u_int32 *)(block + 2*WORD_SIZE) = ((u_int32)prev & 0xFFFFFFFC);
}

void set_used(void *block, bool used)
{
  *(u_int32 *)(block + WORD_SIZE) = \
    (*(u_int32*)(block + WORD_SIZE) & 0xFFFFFFFC) | used;
}


void set_block(void* ptr, size_t size, void *next, void *prev, bool used)
{
  /* writef("Setting : ptr=%x, size=%x, next=%x, prev=%x, used=%d\n", ptr, size, next, prev, used); */
  set_size(ptr, size);
  *(u_int32 *)(ptr + WORD_SIZE) = (u_int32)next | used;  /* Next free block */
  set_previous_free_block(ptr, prev);

  void *header_end = ptr + size - HEADER_SIZE;
  set_size(header_end, size);
  *(u_int32 *)(header_end + WORD_SIZE) = (u_int32)next | used;
  set_previous_free_block(header_end, prev);
}


void write_block(void *block)
{
  writef("Block at %x, size %x, prev free %x, next free %x, used %u\n", \
         block, get_size(block), get_previous_free_block(block), get_next_free_block(block), get_used(block));
}

void write_memory()
{
  writef("First free block at %x\n", first_free_block);
  void* block = (void *)END_OF_KERNEL_HEAP;
  while (block < unallocated_mem) {
    write_block(block);
    block = get_next_block(block);
  }
}


void malloc_install()
{
  /* Allocates a page */
  alloc_frame(get_page(END_OF_KERNEL_HEAP, TRUE, current_directory), FALSE, TRUE);
  unallocated_mem = (void *)(END_OF_KERNEL_HEAP + 0x1000);

  first_free_block = (void *)END_OF_KERNEL_HEAP;
  set_block(first_free_block, 0x00001000, 0, 0, FALSE);  /* Setting size of 4KB: the whole page */
  /* writef("Malloc installed\n"); */
}

void *mem_alloc(size_t size)
{
  /* writef("Allocating a block\n"); */
  size = size + 2*HEADER_SIZE;  /* Account for the two headers */
  if (size % 4) {
    size = (size & 0xFFFFFFFC) + 4;
  }

  void *block = first_free_block;
  /* writef("%c first_free_block: %x, size=%x\n",219,first_free_block,*(u_int32*)first_free_block); */

  while (block && (get_used(block) || get_size(block) < size)) {
    block = get_next_free_block(block);
  }

  if (!block) {
    /* writef("Let's try to allocate a page\n"); */

    void *last_block = unallocated_mem - HEADER_SIZE;
    size_t last_size = get_size(last_block);
    bool free = !get_used(last_block);
    last_block = unallocated_mem - last_size;
    /* writef("Last physical block at %x, size %x, free: %u\n", last_block, last_size, free); */

    int nb_pages;
    void *new_block, *previous_free, *next_free;
    size_t new_size;
    if (free) {
      /* We just extend the last free block */
      nb_pages = ceil_ratio(size - last_size, 0x1000);
      new_block = last_block;
      new_size = last_size + nb_pages * 0x1000;
      previous_free = get_previous_free_block(last_block);
      next_free = get_next_free_block(last_block);
    } else {
      /* We create a new free block, which will be placed at the start of the queue */
      nb_pages = ceil_ratio(size, 0x1000);
      new_block = unallocated_mem;
      new_size = nb_pages * 0x1000;
      previous_free = 0;
      next_free = first_free_block;

      /* Inserting at the start of the queue */
      set_previous_free_block(first_free_block, new_block);
      first_free_block = new_block;
    }
    /* writef("Allocating %u pages\n", nb_pages); */

    for (int i = 0; i < nb_pages; i++) {
      alloc_frame(get_page((u_int32)unallocated_mem, TRUE, current_directory), FALSE, TRUE);
      unallocated_mem += 0x1000;
    }

    set_block(new_block, new_size, next_free, previous_free, FALSE);

    return mem_alloc(size - 2*HEADER_SIZE);  /* Kinda a hack, but fast since first block is good */
  }

  size_t size_block = get_size(block);
  void *previous = get_previous_free_block(block);
  void *next     = get_next_free_block(block);

  if (size_block - size >= WORD_SIZE + 2*HEADER_SIZE) { /*  New block */

    set_block(block,        size,              0,    0,        TRUE);
    set_block(block + size, size_block - size, next, previous, FALSE);

    if (next) {
      set_previous_free_block(next, block + size);
    }
    if (previous) {
      set_next_free_block(previous, block + size);
    } else {
      first_free_block = block + size;
    }
  } else {  /* No new possible block */

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

void mem_free(void *B)
{
  /* Merging conditions: say the block to free is B, contiguous within A - B - C
   * 1) If A is free, with W-> A-> X and C is used, then W-> A-B-> X
   * 2) If C is free, with Y-> C-> Z and A is used, then Y-> B-C-> Z
   * 3) If both are free, then
   *   -> Note: necessarily, W!=Y, X!=Z, W!=X and Y!=Z
   *   -> W = Z and Y = X is an impossible case (W-> A-> X-> C-> W !)
   *   a) if W!= Z and Y!= X then W-> A-B-C-> Z and Y-> X
   *   b) if W!= Z and Y = X then W-> A-B-C-> Z and 0-> X-> first_free_block
   *   c) if W = Z and Y!= X then Y-> A-B-C-> X and 0-> W-> first_free_block
   * 4) If none are free, then 0-> B-> first_free_block */

  /* TODO: free pages when not used anymore? */

  B -= HEADER_SIZE; // Go back to the header
  /* writef("Freeing block %x\n", B); */

  void *A;
  if ((u_int32)B > 0x00800000)
    // TODO: merge this into get_previous_block
    A = get_previous_block(B);
  else
    A = 0;
  void *C = get_next_block(B);
  if ((u_int32)C >= 0x01000000)
    C = 0;

  size_t size = get_size(B);

  if(A && !get_used(A)) { // 1) or 3)
    void* W = get_previous_free_block(A);
    void* X = get_next_free_block(A);
    if(!C || get_used(C)) // 1) : W-> A-B-> X
      set_block(A, get_size(A)+size, X, W, FALSE);
    else { // 3)
      void* Y = get_previous_free_block(C);
      void* Z = get_next_free_block(C);
      if(W!=Z) { // 3)a) or 3)b) : W-> A-B-C-> Z
        set_block(A, get_size(A)+get_size(C)+size, Z, W, FALSE);
        set_previous_free_block(Z,A);
        if(Y!=Z) { // 3)a) : Y-> X
          set_previous_free_block(X, Y);
          set_next_free_block(Y, X);
        } else { // 3)b) : 0-> X=Y-> first_free_block
          set_previous_free_block(X,0);
          set_next_free_block(X,first_free_block);
          first_free_block = X;
        }
      } else { // 3)c) : Y-> A-B-C-> X and 0-> W-> first_free_block
        set_block(A, get_size(A)+get_size(C)+size, X, Y, FALSE);
        set_next_free_block(Y,A);
        set_previous_free_block(W,0);
        set_next_free_block(W, first_free_block);
        first_free_block = W;
      }
    }
  } else { // 2)-4)
    if(C && !get_used(C)) { // 2) : Y-> B-C-> Z
      void* Y = get_previous_free_block(C);
      void* Z = get_next_free_block(C);
      set_block(B, get_size(C)+size, Z, Y, FALSE);
      set_next_free_block(Y,B);
      set_previous_free_block(Z,B);
      if(C==first_free_block)
        first_free_block=B;
    } else { // 4) : 0-> B-> first_free_block
      set_block(B, size, first_free_block, 0, FALSE);
      first_free_block = B;
    }
  }
}
