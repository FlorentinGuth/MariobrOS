#ifndef MEMORY_H
#define MEMORY_H

/** memory.h:
 *  Contains memory-management functions.
 */

#include "types.h"


/** mem_set:
 *  Fills the area with the given byte.
 *
 *  @param str A pointer to the area to fill
 *  @param c   The filler byte
 *  @param len The number of bytes to fill
 */
void mem_set(void* str, u_int8 c, size_t len);

/** mem_copy:
 *  Copy one area into another.
 *
 *  @param dest   The area to overwrite
 *  @param source The area to copy from
 *  @param len    The number of bytes to fill
 */
void mem_copy(void *dest, const void *source, size_t len);

#endif
