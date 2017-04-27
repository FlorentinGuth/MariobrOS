#ifndef MALLOC_H
#define MALLOC_H

#include "types.h"
#include "paging.h"


/**
 * @name malloc_install - Initializes the block structure
 * @return void
 */
void malloc_install();

/**

 * @name mem_alloc - Allocates memory
 * @param size - the number of bytes to allocate
 * @return a pointer to the free memory, or 0 if there's not enough space
 */
void *mem_alloc(size_t size);

/**
 * @name mem_alloc_aligned - Allocates aligned memory
 * @param size - the number of bytes to allocate
 * @param alignment - alignment of the block
 * @return a pointer, whose value is a multiple of alignment
 */
void *mem_alloc_aligned(size_t size, unsigned int alignment);

/**
 * @name mem_free - Frees used memory
 * @param ptr - Pointer to the used memory
 * @return void
 */
void mem_free(void* ptr);


/**
 * @name write_memory - Print the memory structure to the framebuffer
 * @return void
 */
void write_memory();

#endif
