#include "fs_inter.h"

fdt_e* fdt = 0;
u_int32 fdt_tot  = 0;
u_int32 fdt_curr = 0;


fd openfile(string path, open_flag oflag, u_int16 fperm)
{
  u_int32 inode = find_inode(path, 2);
  if(oflag == O_EXCL && inode) {
    return -1;
  }
  if(fdt_curr < fdt_tot) {
    fdt[fdt_curr].inode = inode;
    fdt[fdt_curr].pos   = 0; // TODO Deal according to the flags
    fdt[fdt_curr].mode  = fperm;
    fdt_curr++;
    return fdt_curr - 1;
  } else {
    return -2; // TODO allocate more space
  }
}

void fs_inter_install()
{
  fdt = (void*) mem_alloc(256 * sizeof(fdt_e));
  fdt_tot = 256;
  fdt_curr = 0;
}
