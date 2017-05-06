#ifndef BITSET_H
#define BITSET_H

#include "types.h"


/**
 * A bitset is a boolean array, optimized to take as little place in memory
 * as possible.
 */

typedef struct bitset {
  u_int32 length;  /* Number of bits in the structure */
  u_int32 *bits;
} bitset_t;

bitset_t empty_bitset(u_int32 lenght);

void set_bit(bitset_t b, u_int32 bit, bool value);
bool get_bit(bitset_t b, u_int32 bit);

u_int32 first_false_bit(bitset_t b);

#endif
