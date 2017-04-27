#include "types.h"
#include "kheap.h"
#include "printer.h"
#include "error.h"


/* TODO: choose a type between void*, unsigned int, something else (long?) */

/* The heap pointer */
unsigned int brk = 0;


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
  if (brk == 0) {
    brk = (unsigned int)END_OF_KERNEL_LOCATION;
  }

  /* Align only if it is not already page-aligned */
  if (align && (brk & 0xFFFFF000)) {
    brk &= 0xFFFFF000;
    brk += 0x1000;
  }

  /* Detect if there's no space anymore */
  if (brk + size > END_OF_KERNEL_HEAP) {
    throw("Out of Memory on kernel heap");
  }

  if (physical_address) {
    *physical_address = brk;
  }

  unsigned int address = brk;
  brk += size;
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
