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

/**
 *  @name set_bit - Sets or clears one bit of the bitset
 *  @param b      - The bitset
 *  @param bit    - The bit to set or clear
 *  @param value  - The new value
 */
void set_bit(bitset_t b, u_int32 bit, bool value);

/**
 *  @name get_bit - Gets one bit from the bitset
 *  @param b      - The bitset
 *  @param bit    - The bit to get
 */
bool get_bit(bitset_t b, u_int32 bit);

/**
 *  @name first_false_bit - Gets the position of the first clear bit
 *  @param b              - The bitset
 *  @return               - The position, or -1 [2^32] if none was found
 */
u_int32 first_false_bit(bitset_t b);

/**
 *  @name param_ffb - Same as first_false_bit, but with a starting bit
 */
u_int32 param_ffb(bitset_t b, u_int32 first);
  
#endif
