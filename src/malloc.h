#ifndef MALLOC_H
#define MALLOC_H

#include "types.h"
#include "paging.h"


void *first_free_block;
void *unallocated_mem;

/**
 *  @name malloc_install - Initializes the block structure
 *  @return void
 */
void malloc_install();

/**
 *  @name mem_alloc - Allocates memory
 *  @param size     - The number of bytes to allocate
 *  @return void*   - A pointer to the free memory, or 0 if there's not enough space
 */
void *mem_alloc(size_t size);

/**
 *  @name mem_alloc_aligned - Allocates aligned memory
 *  @param size             - The number of bytes to allocate
 *  @param alignment        - Alignment of the block
 *  @return void*           - A pointer, whose value is a multiple of alignment
 */
void *mem_alloc_aligned(size_t size, unsigned int alignment);

/**
 *  @name mem_free - Frees used memory
 *  @param ptr     - Pointer to the used memory
 *  @return void
 */
void mem_free(void* ptr);


/**
 *  @name log_memory - Logs the heap structure
 *  @return void
 */
void log_memory();

#endif
