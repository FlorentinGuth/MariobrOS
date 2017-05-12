#include "fs_inter.h"

fdt_e* fdt = 0;
fd fdt_tot  = 0;


fd openfile(string path, u_int8 oflag, u_int16 fperm)
{
  u_int32 inode = find_inode(path, 2);
  if(((oflag & O_EXCL) && inode) || !( (oflag & O_CREAT) || inode) ) {
    return -1;
  }
  u_int32 f = (u_int32) (-1);
  for(s_int32 i = 0; i < fdt_tot && fdt[i].inode; i++) {
    if(!fdt[i].inode) {
      f = i;
      break;
    }
  }
  if(f == (u_int32) (-1))
    return -2; // TODO allocate more space

  if(!inode) { // O_CREAT flag is set because of the first test
    /* std_buf has been set by find_inode to the content of the data block of
     * the parent directory, so its first directory entry is "." */
    list_t l = str_split(path, '/', FALSE);
    while(l->tail) {
      mem_free((void*) l->head);
      pop(&l);
    }
    inode = create_file( ((dir_entry*) std_buf)->inode, (string) l->head, \
                         TYPE_FILE | fperm, FILE_REGULAR);
    mem_free((void*) l->head);
    pop(&l);
    if(!inode) {
      return -3;
    }
  }

  if(oflag & O_TRUNC) {
    erase_file_data(inode);
  }

  if(oflag & O_APPEND) {
    set_inode(inode, std_inode);
    fdt[f].pos = std_inode->size_low;
  } else {
    fdt[f].pos = 0;
  }
  fdt[f].inode = inode;
  fdt[f].mode = oflag & 0x3; // Only RDONLY, WRONLY or RDWR
  return f;
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
