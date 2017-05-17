#include "lib.h"
#include "string.h"

int main()
{
  writef("A\n");
  fd f = open("/test1/A_new_hope", O_CREAT | O_RDWR, PERM_ALL);
  writef("B\n");
  string buf = "Hello world!\n";
  write(f, (void*)buf, 0, str_length(buf));
  writef("C\n");
  u_int8 out[100];
  lseek(f, 6, SEEK_SET);
  writef("D\n");
  for(int i = 0; i < 100; i++) {
    out[i] = 0;
  }
  writef("E\n");
  read(f, (void*) out, 0, str_length(buf));
  writef("Out: %s\n", out);
  close(f);
  return 0;
}
