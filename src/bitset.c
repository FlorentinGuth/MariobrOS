#include "bitset.h"


#define INDEX_FROM_BIT(bit)  (bit / 32)
#define OFFSET_FROM_BIT(bit) (bit % 32)


void set_bit(bitset_t b, u_int32 bit, bool value)
{
  b.bits[INDEX_FROM_BIT(bit)] |= (value << OFFSET_FROM_BIT(bit));
}

bool get_bit(bitset_t b, u_int32 bit)
{
  return b.bits[INDEX_FROM_BIT(bit)] & (0x1 << OFFSET_FROM_BIT(bit));
}


u_int32 first_false_bit(bitset_t b)
{
  for (u_int32 i = 0; i < INDEX_FROM_BIT(b.length); i++) {
    if (b.bits[i] != 0xFFFFFFFF) {  /* Nothing free, exit early. */
      /* At least one bit is free here. */
      for (u_int32 j = 0; j < 32; j++) {
        if (!(b.bits[i] & (0x1 << j)))
          return 32*i + j;
      }
    }
  }

  /* No false bits */
  return (u_int32)(-1);
}
