#include "lib.h"


int main()
{
  void *ptrs[1000];

  for (int i = 0; i < 1000; i++)
    ptrs[i] = malloc(1);
  printf("Survived allocation\n");
  for (int i = 0; i < 1000; i++)
    free(ptrs[i]);
  printf("I survived!\n");

  return 0;
}
