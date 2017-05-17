#include "lib.h"
#include "string.h"

int main()
{
  fd f = open("/test1/A new hope", O_CREAT | O_RDWR, PERM_ALL);
  string buf = "Hello world!\n";
  write(f, (void*)buf, 0, str_length(buf));
  u_int8 out[100];
  lseek(f, 6, SEEK_SET);
  for(int i = 0; i < 100; i++) {
    out[i] = 0;
  }
  read(f, (void*) out, 0, str_length(buf));
  printf("Out: %s\n", out);
  close(f);
  return 0;
}
