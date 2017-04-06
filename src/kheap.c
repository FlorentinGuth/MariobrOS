#include "types.h"
#include "kheap.h"
#include "printer.h"


/* TODO: chose a type between void*, unsigned int, something else (long?) */

/* The heap pointer */
unsigned int sbrk = 0;


/** kmalloc_internal:
 *  Allocates memory on the kernel heap. This is not meant to be freed.
 *
 *  @param size             The size of the memory to allocate (in bytes)
 *  @param align            Whether to page-align (4KB)
 *  @param physical_address If non-null, the value pointed will be the physical address of the allocated memory
 *  @return                 The virtual address of the allocated memory
 */
unsigned int kmalloc_internal(unsigned int size, bool align, unsigned int *physical_address)
{
  /* Initialization */
  if (sbrk == 0) {
    sbrk = (unsigned int)END_OF_KERNEL_LOCATION;
  }

  /* Align only if it is not already page-aligned */
  if (align && (sbrk & 0xFFFFF000)) {
    sbrk &= 0xFFFFF000;
    sbrk += 0x1000;
  }

  /* Detect if there's no space anymore */
  if (sbrk + size > PAGE_TABLES_LOCATION) {
    write("\nERROR: Out of Memory on kernel heap\n");
    for(;;);  /* Endless loop, TODO: add a proper error function somewhere */
  }

  if (physical_address) {
    *physical_address = sbrk;
  }

  unsigned int address = sbrk;
  sbrk += size;
  return address;
}

unsigned int kmalloc_aligned(unsigned int size)
{
  return kmalloc_internal(size, TRUE, 0);
}

unsigned int kmalloc_physical(unsigned int size, unsigned int *physical_address)
{
  return kmalloc_internal(size, FALSE, physical_address);
}

unsigned int kmalloc_physical_aligned(unsigned int size, unsigned int *physical_address)
{
  return kmalloc_internal(size, TRUE, physical_address);
}

unsigned int kmalloc(unsigned int size)
{
  return kmalloc_internal(size, FALSE, 0);
}
