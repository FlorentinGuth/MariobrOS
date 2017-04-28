#ifndef MALLOC_H
#define MALLOC_H

#include "types.h"
#include "paging.h"


/**
 *  @name malloc_install -
 *  Initializes the memory to allow dynamic allocation and freeing.
 */
void malloc_install();

/**
 *  @name mem_alloc - Allocates memory using paging. 
 *  This is always 4 bytes-aligned.
 *
 *  @param size - The number of bytes to allocate.
 *  @return     - A pointer to the free memory.
 */
void *mem_alloc(size_t size);

/**
 *  @name free - Frees previously mallocated memory.
 *
 *  @param ptr - A pointer to the memory to free.
 */
void mem_free(void* ptr);


void write_memory();

#endif
