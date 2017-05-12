#include "fs_inter.h"

fdt_e* fdt = 0;
fd fdt_tot  = 0;


fd openfile(string path, open_flag oflag, u_int16 fperm)
{
  u_int32 inode = find_inode(path, 2);
  if(oflag == O_EXCL && inode) {
    return -1;
  }
  for(s_int32 i = 0; i < fdt_tot && fdt[i].inode; i++) {
    if(!fdt[i].inode) {
      fdt[i].inode = inode;
      fdt[i].pos   = 0; // TODO Deal according to the flags
      fdt[i].mode  = fperm;
      return i;
    }
  }
  return -2; // TODO allocate more space
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
u_int32 read(fd f, u_int16* buffer, u_int32 offset, u_int32 length)
{
  return 0;
}

void close(fd f)
{
  fdt[f].inode = 0;
}

void fs_inter_install()
{
  fdt = (void*) mem_alloc(256 * sizeof(fdt_e));
  fdt_tot = 256;
}
