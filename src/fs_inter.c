#include "fs_inter.h"

fdt_e* fdt = 0;
u_int32 fdt_size = 0;
u_int32 fdt_num = 0;


fd openfile(string path, u_int8 oflag, u_int16 fperm)
{
  u_int32 inode = find_inode(path, 2);
  if(!( (oflag & O_CREAT) || inode) ) {
    /* kloug(100, "File %s does not exist\n", path); */
    return 0;
  }
  if((oflag & O_EXCL) && inode) {
    /* kloug(100, "File %s already exists\n", path); */
    return 0;
  }

  u_int32 i;
  for(i = 0; i < fdt_size && fdt[i].inode; i++);
  if(i == fdt_size) { // fdt is full, so it shall double in size
    kloug(100,"Expanding File Descriptor Table (from size %u)\n", fdt_size);
    fdt_e* tmp = (void*) mem_alloc(2 * fdt_size * sizeof(fdt_e));
    for(u_int32 j = 0; j < fdt_size; j++) {
      tmp[j] = fdt[j];
    }
    for(u_int32 j = fdt_size; j < 2 * fdt_size; j++) {
      tmp[j].inode = 0;
    }
    mem_free(fdt);
    fdt = tmp;
    i = fdt_size;
    fdt_size *= 2;
  }
  fd f = (void*) mem_alloc(sizeof(void*));
  *f = i;
  fdt_num++;

  if(!inode) { // O_CREAT flag is set because of the first test
    /* std_buf has been set by find_inode to the content of the data block of
       the parent directory, so its first directory entry is "." */
    kloug(100, "Creating file %s\n", path);
    list_t l = str_split(path, '/', FALSE);
    u_int32 parent = ((dir_entry*) std_buf)->inode;
    while(l->tail) {
      mem_free((void*) l->head);
      pop(&l);
    }
    inode = create_file(parent, (void*) l->head, TYPE_FILE | fperm, \
                        FILE_REGULAR);
    /* kloug(100, "Inode for %s is %u\n", path, inode); */
    mem_free((void*) l->head);
    pop(&l);
    if(!inode) {
      kloug(100, "File creation for %s failed", path);
      return 0;
    }
  }
  if(oflag & O_TRUNC) {
    erase_file_data(inode);
  }
  set_inode(inode, std_inode);
  if(oflag & O_APPEND) {
    fdt[*f].pos = std_inode->size_low;
  } else {
    fdt[*f].pos = 0;
  }
  fdt[*f].size = std_inode->size_low;
  fdt[*f].inode = inode;
  fdt[*f].mode = oflag & 0x3; // Only RDONLY, WRONLY or RDWR
  fdt[*f].this = f;
  /* The "this" field allows to move file descriptors when the fdt is shrinked
   * and also allows to prevent a closed file descriptor from referring to
   * another file.
   */
  /* kloug(100, "File descriptor n°%u created\n", *f); */
  return f;
}

fd openker(string path)
{
  return openfile(path, O_RDWR, 0xffff);
}

u_int32 read(fd f, u_int8* buffer, u_int32 offset, u_int32 length)
{
  if(!f || *f > fdt_size || fdt[*f].this != f || !fdt[*f].inode) {
    writef("Invalid file descriptor\n");
    return -1; // Invalid file descriptor
  }
  if(!(fdt[*f].mode & O_RDONLY)) {
    writef("Could not write on file without the appropriate flag\n");
    return -2; // No permission
  }
  if(fdt[*f].pos >= fdt[*f].size) {
    return 0;
  }
  if(fdt[*f].pos + length > fdt[*f].size) {
    length = fdt[*f].size - fdt[*f].pos;
  }
  u_int32 done = read_inode_data(fdt[*f].inode, buffer + offset, fdt[*f].pos,\
                                 length);
  fdt[*f].pos += done;
  return done;
}

u_int32 write(fd f, u_int8* buffer, u_int32 offset, u_int32 length)
{
  if(!f || *f > fdt_size || fdt[*f].this != f || !fdt[*f].inode) {
    writef("Invalid file descriptor\n");
    return -1; // Invalid file descriptor
  }
  if(!(fdt[*f].mode & O_WRONLY)) {
    writef("Could not write on file without the appropriate flag\n");
    return -2; // No permission
  }
  if(!length) {
    return 0;
  }
  u_int32 to_use = 1 + (fdt[*f].pos + length - 1) / block_size;
  u_int32 used;
  if(fdt[*f].size) {
    used = 1 + (fdt[*f].size - 1) / block_size;
  } else {
    used = 0;
  }

  u_int32 alloc  = prepare_blocks(fdt[*f].inode, used, to_use);
  if(to_use > used && alloc != to_use - used) {
    writef("The block allocation failed\n");
    writef("Could only allocate %u instead of %u\n", alloc, to_use - used);
    return -3; // All the blocks could not be allocated
  }
  u_int32 written = 0;
  u_int32 done = 0;
  while(written != length) {
    done = write_inode_data(fdt[*f].inode, buffer + offset, fdt[*f].pos,   \
                            length - written);
    if(!done) { // No data was written this time, so it won't evolve
      break;
    }
    offset += done;
    fdt[*f].pos += done;
    written += done;
  }
  if(fdt[*f].pos > fdt[*f].size) {
    fdt[*f].size = fdt[*f].pos;
    // std_inode was set by write_inode_data or by prepare_block
    std_inode->size_low = fdt[*f].size;
    update_inode(fdt[*f].inode, std_inode);
  }

  return written;
}

void fstat(fd f, stats* s)
{
  if(!f) {
    writef("Invalid file descriptor\n");
  }
  set_inode(fdt[*f].inode, std_inode);
  s->st_ino = fdt[*f].inode;
  s->st_kind = std_inode->type >> 12;
  s->st_perm = std_inode->type & 0x0FFF;
  s->st_nlink = std_inode->hard_links;
  s->st_size = std_inode->size_low;
}

u_int32 lseek(fd f, s_int32 offset, u_int8 seek)
{
  if(seek != SEEK_SET && seek != SEEK_CUR && seek != SEEK_END) {
    writef("Invalid seek command\n");
    return 0;
  }
  if(seek & SEEK_SET) {
    if(offset < 0) {
      fdt[*f].pos = 0;
    } else if((u_int32) offset > fdt[*f].size) {
      fdt[*f].pos = fdt[*f].size;
    } else {
      fdt[*f].pos = (u_int32) offset;
    }
  } else if(seek & SEEK_CUR) {
    if(offset < 0 && (u_int32) (-offset) > fdt[*f].pos) {
      fdt[*f].pos = 0;
    } else if(offset > 0 && (u_int32) offset + fdt[*f].pos > fdt[*f].size) {
      fdt[*f].pos = fdt[*f].size;
    } else {
      fdt[*f].pos += (u_int32) offset;
    }
  } else { // seek == SEEK_END
    if(offset < 0) {
      fdt[*f].pos = fdt[*f].size;
    } else if((u_int32) offset > fdt[*f].size) {
      fdt[*f].pos = 0;
    } else {
      fdt[*f].pos -= (u_int32) offset;
    }
  }
  return fdt[*f].pos;
}

void close(fd f)
{
  if(!f || !fdt[*f].inode) {
    return;
  }
  fdt[*f].inode = 0;
  fdt[*f].this = 0;
  mem_free(f);
  fdt_num--;
  if(fdt_num > 64 && fdt_num < fdt_size / 4) { // Shrink the fdt
    kloug(100, "Shrinking File Descriptor Table (from size %u)\n", fdt_size);
    fdt_e* tmp = (void*) mem_alloc((fdt_size / 2) * sizeof(fdt_e));
    bool copy = TRUE;
    u_int32 j;
    for(u_int32 i = 0; i < fdt_size / 4; i++) {
      if(fdt[i].inode || !copy) { // Simple transfer from fdt to tmp
        tmp[i] = fdt[i];
      } else { // Move an fd entry within the table
        for(j = fdt_size - 1; j > i; j--) {
          if(fdt[j].inode) { // Move fd entry j to i
            tmp[i] = fdt[j];
            *(tmp[i].this) = i; // Change associated file descriptor
            fdt[j].inode = 0;
            break;
          }
        }
        if(j == fdt_size) {
          writef("STOP: %u\n",j);
          copy = FALSE; // No more fd to move
        }
      }
    }
    mem_free(fdt);
    fdt = tmp;
    fdt_size /= 2;
  }
}

void fs_inter_install()
{
  /* The fdt is a dynamic array of file descriptors.
   * It doubles in size when it is full, and shrinks by half when its
   * ocupation rate is below 1/4. This ensures a constant amortized cost per
   * operation of creation and deletion of file descriptors
   */
  fdt_size = 256;
  fdt = (void*) mem_alloc(fdt_size * sizeof(fdt_e));
  for(int i = 0; i < 256; i++) {
    fdt[i].inode = 0;
  }
}
