#ifndef KHEAP_H
#define KHEAP_H


/** kmalloc*:
 *  Allocates memory on the kernel heap. This is not meant to be freed.
 *
 *  @param size             The size of the memory to allocate (in bytes)
 *  @param align            Whether to page-align (4KB)
 *  @param physical_address If non-null, the value pointed will be the physical address of the allocated memory
 *  @return                 The virtual address of the allocated memory
 */

unsigned int kmalloc_aligned(unsigned int size);
unsigned int kmalloc_physical(unsigned int size, unsigned int *physical_address);
unsigned int kmalloc_physical_aligned(unsigned int size, unsigned int *physical_address);
unsigned int kmalloc(unsigned int size);

#endif
