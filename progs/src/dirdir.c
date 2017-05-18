#include "lib.h"

int main()
{
  string name = "t0";
  u_int32 inode = 2;
  for(int i = 0; i < 1000; i++) {
    mkdir((void*) name, inode);
    inode = find_dir((void*) name, inode);
    name[1] = 49 + (i % 10);
  }
  return 0;
}
