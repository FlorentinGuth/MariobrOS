#include "lib.h"
#include "string.h"

int main()
{
  fd f = open("/data/to_erase", O_CREAT, PERM_ALL);
  close(f);
  f = open("/data/to_erase", O_EXCL, PERM_ALL);
  if(!f) {
    writef("to_erase has well been created\n");
  }
  close(f);
  f = open("/data/to_erase", O_RDWR, PERM_ALL);
  string buf = "I shall disappear\n";
  write(f, (void*)buf, 0, str_length(buf));
  close(f);
  f = open("/data/to_erase", O_APPEND | O_WRONLY, PERM_ALL);
  buf = "Nonetheless, I am.";
  write(f, (void*)buf, 0, str_length(buf));
  
  u_int8 out[256] = {0};
  writef("The next step should not work: ");
  writef("%u bytes were written\n", read(f, out, 0, 256));
  close(f);
  f = open("/data/to_erase", O_RDONLY, PERM_ALL);
  read(f, out, 0, 256);
  writef("Contents: %s", out);
  close(f);
  rm("/data/to_erase", 2, FALSE);
  writef(".. no more");
  return 0;
}
