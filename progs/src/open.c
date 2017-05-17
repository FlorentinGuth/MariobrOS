#include "lib.h"
#include "string.h"

int main()
{
  printf("A\n");
  fd f = open("/test1/A_new_hope", O_CREAT | O_RDWR, PERM_ALL);
  printf("B\n");
  string buf = "Hello world!\n";
  write(f, (void*)buf, 0, str_length(buf));
  printf("C\n");
  u_int8 out[100];
  lseek(f, 6, SEEK_SET);
  printf("D\n");
  for(int i = 0; i < 100; i++) {
    out[i] = 0;
  }
  printf("E\n");
  read(f, (void*) out, 0, str_length(buf));
  printf("Out: %s\n", out);
  close(f);
  return 0;
}
