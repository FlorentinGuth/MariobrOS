#include "math.h"


unsigned int floor_ratio(unsigned int a, unsigned int b)
{
  return (unsigned int)(a/b);
}

unsigned int ceil_ratio(unsigned int a, unsigned int b)
{
  unsigned int q = a / b;
  if (a == q * b)
    return q;
  return q + 1;
}
