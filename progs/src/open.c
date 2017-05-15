#include "lib.h"


int main()
{
  /* fd f =  */open("froude", O_RDWR | O_CREAT, PERM_USER_R | PERM_USER_W | PERM_USER_E);
  printf("Opened\n");
  /* close(f); */
  /* printf("Closed\n"); */
  return 0;
}
