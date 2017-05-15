#include "lib.h"


int main()
{
  fd f = open("test2/autre", 0xde, 0xbeef);
  printf("Opened\n");
  close(f);
  printf("Closed\n");
  return 0;
}
