#include "bitset.h"
#include "malloc.h"
#include "memory.h"
#include "math.h"
#include "logging.h"


#define INDEX_FROM_BIT(bit)  (bit / 32)
#define OFFSET_FROM_BIT(bit) (bit % 32)


bitset_t empty_bitset(u_int32 length)
{
  bitset_t b;

  length = ceil_multiple(length, 32);
  /* The length should be divisible by 32 to avoid strange cases */
  b.length = length;

  b.bits = mem_alloc(length / 8);      /* Each byte store 8 bits */
  mem_set(b.bits, FALSE, length / 8);

  return b;
}


void set_bit(bitset_t b, u_int32 bit, bool value)
{
  b.bits[INDEX_FROM_BIT(bit)] \
    = (b.bits[INDEX_FROM_BIT(bit)] & ~(1 << OFFSET_FROM_BIT(bit)))  \
    | (value << OFFSET_FROM_BIT(bit));
}

bool get_bit(bitset_t b, u_int32 bit)
{
  return !!(b.bits[INDEX_FROM_BIT(bit)] & (0x1 << OFFSET_FROM_BIT(bit)));
}


u_int32 first_false_bit(bitset_t b)
{
  for (u_int32 i = 0; i < INDEX_FROM_BIT(b.length); i++) {
    if (b.bits[i] != 0xFFFFFFFF) {
      /* At least one bit is free here. */
      for (u_int32 j = 0; j < 32; j++) {
        if (!(b.bits[i] & (0x1 << j))) {
          /* kloug(100, "Bit found at %x\n", 32*i+j); */
          return 32*i + j;
        }
      }
    }
  }

  /* No false bit */
  return (u_int32)(-1);
}

u_int32 param_ffb(bitset_t b, u_int32 first)
{
  u_int8 j = OFFSET_FROM_BIT(first);
  for (u_int32 i = first / 32; i < INDEX_FROM_BIT(b.length); i++) {
    if (b.bits[i] != 0xFFFFFFFF) {
      /* At least one bit is free here. */
      for (; j < 32; j++) {
        if (!(b.bits[i] & (1 << j))) {
          /* kloug(100, "Bit found at %x\n", 32*i+j); */
          return 32*i + j;
        }
      }
      j = 0;
    }
  }

  /* No false bit */
  return (u_int32)(-1);
}
