#include "lib.h"
#include "string.h"

int main()
{
  fd f = open("/test1/A_new_hope", O_CREAT | O_RDWR, PERM_ALL);
  string buf = "Hello world!\n";
  write(f, (void*)buf, 0, str_length(buf));
  u_int8 out[256];
  lseek(f, 6, SEEK_SET);
  for(int i = 0; i < 256; i++) {
    out[i] = 0;
  }
  read(f, (void*) out, 0, str_length(buf));
  writef("Out: %s\n", out);
  close(f);

  f = open("/data/ascii", O_CREAT | O_WRONLY, PERM_ALL);
  for(int i = 0; i < 256; i++) {
    out[i] = i;
  }
  write(f, (void*) out, 0, 256);
  close(f);
  return 0;
}
