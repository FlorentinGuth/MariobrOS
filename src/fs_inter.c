#include "fs_inter.h"

fdt_e* fdt = 0;
fd fdt_tot = 0;


fd openfile(string path, u_int8 oflag, u_int16 fperm)
{
  u_int32 inode = find_inode(path, 2);
  if(((oflag & O_EXCL) && inode) || !( (oflag & O_CREAT) || inode) ) {
    writef("Inode creation failed\n");
    return -1;
  }
  s_int32 f = 0;

  for(; f < fdt_tot && fdt[f].inode; f++);
  if(f == fdt_tot) {
    writef("No room !\n");
    return -2; // TODO allocate more space
  }

  if(!inode) { // O_CREAT flag is set because of the first test
    /* std_buf has been set by find_inode to the content of the data block of
     * the parent directory, so its first directory entry is "." */
    writef("Creating inode\n");
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
    writef("Truncating file\n");
    erase_file_data(inode);
  }

  set_inode(inode, std_inode);
  if(oflag & O_APPEND) {
    fdt[f].pos = std_inode->size_low;
  } else {
    fdt[f].pos = 0;
  }
  fdt[f].size = std_inode->size_low;
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
  if(fdt[f].pos >= fdt[f].size) {
    return 0;
  }
  if(fdt[f].pos + length > fdt[f].size) {
    length = fdt[f].size - fdt[f].pos;
  }
  u_int32 done = read_inode_data(fdt[f].inode, &(buffer[offset]), fdt[f].pos,\
                                 length);
  fdt[f].pos += done;
  return done;
}

u_int32 write(fd f, u_int8* buffer, u_int32 offset, u_int32 length)
{
  if(!fdt[f].inode) {
    writef("Bad file descriptor\n");
    return -1; // Invalid file descriptor
  }
  if(!(fdt[f].mode & O_WRONLY)) {
    writef("Could not write on file without the appropriate flag\n");
    return -2; // No permission
  }
  if(!length) {
    return 0;
  }
  u_int32 to_use = 1 + (fdt[f].pos + length - 1) / block_size;
  u_int32 used;
  if(fdt[f].size) {
    used = 1 + (fdt[f].size - 1) / block_size;
  } else {
    used = 0;
  }

  u_int32 alloc  = prepare_blocks(fdt[f].inode, used, to_use);
  if(to_use > used && alloc != to_use - used) {
    writef("The block allocation failed\n");
    writef("Could only allocate %u instead of %u\n", alloc, to_use - used);
    return -3; // All the blocks could not be allocated
  }
  writef("Allocation succeeded\n");
  u_int32 written = 0;
  u_int32 done = 0;
  while(written != length) {
    writef("?, %u\n", fdt[f].inode);
    done = write_inode_data(fdt[f].inode, &(buffer[offset]), fdt[f].pos,   \
                            length - written);
    writef("!\n");
    if(!done) { // No data was written this time, so it won't evolve
      writef("Break");
      break;
    }
    offset += done;
    fdt[f].pos += done;
    written += done;
  }
  if(fdt[f].pos > fdt[f].size) {
    fdt[f].size = fdt[f].pos;
    // std_inode was set by write_inode_data or by prepare_block
    std_inode->size_low = fdt[f].size;
    writef("Updating...");
    update_inode(fdt[f].inode, std_inode);
    writef("Done\n");
  }
  writef("Written: %u\n", written);
  return written;
}

void close(fd f)
{
  fdt[f].inode = 0;
}

void fs_inter_install()
{
  fdt = (void*) mem_alloc(256 * sizeof(fdt_e));
  for(int i = 0; i < 256; i++) {
    fdt[i].inode = 0;
  }
  fdt_tot = 256;

  fd test = openfile("/test2/filetest", O_RDWR, 0);
  u_int8 *buffer = mem_alloc(4097);
  for(int i = 0; i < 4097; i++) {
    buffer[i] = (2*i + 1) % 256;
  }
  write(test, buffer, 0, 4097);
  for(int i = 0; i < 4097; i++) {
    buffer[i] = 0;
  }
  close(test);
  test = openfile("/test2/filetest", O_RDONLY, 0);

  read(test, buffer, 0, 12);
  for(int i = 0; i < 20; i++) {
    writef("%u: %u\n", i, buffer[i]);
  }
}
