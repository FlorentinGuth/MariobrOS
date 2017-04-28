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


unsigned int floor_multiple(unsigned int n, unsigned int k)
{
  return k * (n / k);
}

unsigned int ceil_multiple(unsigned int n, unsigned int k)
{
  if (n % k) {
    return n - (n % k) + k;
  } else {
    return n;
  }
}
