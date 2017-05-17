#include "lib.h"
#include "string.h"

int main()
{
  printf("What?\n");
  fd f = open("/test1/A_new_hope", O_CREAT | O_RDWR, PERM_ALL);
  printf("What?\n");
  string buf = "Hello world!\n";
  write(f, (void*)buf, 0, str_length(buf));
  printf("What?\n");
  u_int8 out[100];
  lseek(f, 6, SEEK_SET);
  printf("What?\n");
  for(int i = 0; i < 100; i++) {
    out[i] = 0;
  }
  printf("What?\n");
  read(f, (void*) out, 0, str_length(buf));
  printf("Out: %s\n", out);
  close(f);
  return 0;
}
