#include "lib.h"

unsigned int str_length(string s)
{
  int pos = 0;
  for (; s[pos] != '\0'; pos++);
  return pos;
}

int main()
{
  fd f = open("This is SPARTA!!", O_CREAT | O_RDWR, PERM_ALL);
  string buf = "Hello world!";
  write(f, (void*)buf, 0, str_length(buf));
  u_int8 out[100];
  lseek(f, 0, SEEK_SET);
  for(int i = 0; i < 100; i++) {
    out[i] = 0;
  }
  read(f, (void*) out, 0, str_length(buf));
  printf("Out: %s\n", out);
  return 0;
}
