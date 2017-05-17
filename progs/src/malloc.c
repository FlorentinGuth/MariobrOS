#include "lib.h"


int main()
{
  void *ptrs[1000];

  writef("Tests incoming\n");
  for (int i = 0; i < 1000; i++) {
    ptrs[i] = malloc(0x1000);
    *(u_int8 *)ptrs[i] = 0;
  }
  writef("Survived allocation\n");
  for (int i = 0; i < 1000; i++)
    free(ptrs[i]);
  writef("I survived!\n");

  return 0;
}
