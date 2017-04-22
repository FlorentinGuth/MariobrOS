#include "math.h"


unsigned int floor_ratio(unsigned int a, unsigned int b)
{
  unsigned int q = a / b;
  return q;
}

unsigned int ceil_ratio(unsigned int a, unsigned int b)
{
  unsigned int q = a / b;
  if (a == q * b)
    return q;
  else
    return q + 1;
}
