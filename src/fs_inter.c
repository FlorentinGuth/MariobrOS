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

  set_inode(inode, std_inode);
  if(oflag & O_APPEND) {
    fdt[f].pos = std_inode->size_low;
  } else {
    fdt[f].pos = 0;
  }
  fdt[f].size = std_inde->size_low;
  fdt[f].inode = inode;
  fdt[f].mode = oflag & 0x3; // Only RDONLY, WRONLY or RDWR
  return f;
}

u_int32 read(fd f, u_int8* buffer, u_int32 offset, u_int32 length)
{
  if(!fdt[f].inode) {
    return -1; // Invalid file descriptor
  }
  if(!(fdt[f].mode & O_RDONLY)) {
    return -2; // No permission
  }
  fdt[f].pos += offset;
  if(fdt[f].pos >= fdt[f].size) {
    return 0;
  }
  if(fdt[f].pos + length > fdt[f].size) {
    length = fdt[f].size - fdt[f].pos;
  }
  u_int32 done = read_inode_data(fdt[f].inode, buffer, fdt[f].pos, length);
  fdt[f].pos += done;
  return done;
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
