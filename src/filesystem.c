#include "filesystem.h"
#include "logging.h"

/* lba n means sector n, with sector_size = 0x200 = 512 bytes. The volume starts
 * at 1M = 0x10000 in memory. Thus, lba n is the address 512 * n from the 
 * beginning  of the volume, that is at the physical address 0x10000 + n * 0x200
 * The superpblock is at lba 2, so 0x10400 is the position of block 0.
 * Each block is 2048 = 256 * 8 bytes wide.
 */


superblock_t *spb = 0; // Superblock address
bgp_t *bgpt = 0; // Block group descriptor table
u_int32 block_factor = 0; // Only used in the definition of the BLOCK macro
u_int32 block_size = 0; // block size in bytes
u_int32 bgpt_size = 0;
u_int8 *std_buf = 0;
/* The above standard buffer is set up with the size of a block using mem_alloc.
 * Its purpose is to give access to memory within a function, without a call
 * to mem_alloc each time such a buffer is needed.
 * Warning: its content may be modified by any of the following elementary
 * functions, so do not use it while calling such functions. */
inode_t *std_inode = 0;
/* Similar to std_buf, but for inodes */

/**
 *  @name BLOCK - Used in readPIO, readLBA and writeLBA to access a block
 *  @param n    - The block number to access
 */
#define BLOCK(n) (block_factor*(n))


void update_block(u_int8* buffer, u_int32 ofs, u_int32 length, u_int32 address)
{
  address += block_factor * (ofs / block_size);
  ofs %= block_size;

  /* writef("Updating BLOCK %u with offset %u and length %u\n", address, ofs, length); */
  readLBA(address, block_factor, std_buf);
  for(u_int32 pos = ofs; pos < ofs + length; pos++) {
    std_buf[pos] = buffer[pos-ofs];
  }
  writeLBA(address, block_factor, std_buf);
}


void set_inode(u_int32 inode, inode_t *buffer)
{
  u_int32 block_group = (inode-1) / spb->inode_per_group;
  u_int32 index = (inode-1) % spb->inode_per_group;
  /* writef("Finding inode %u (block_group %u, index %u)\n", inode, block_group, index); */
  readPIO(BLOCK(bgpt[block_group].inode_table_address), \
          index*sizeof(inode_t), sizeof(inode_t), (void*) buffer);
}

void update_inode(u_int32 inode, inode_t *buffer)
{
  u_int32 block_group = (inode-1) / spb->inode_per_group;
  u_int32 index = (inode-1) % spb->inode_per_group;
  /* writef("Updating inode %u (block group %u, index %u)\n",inode, block_group, index); */
  update_block((u_int8*) buffer, index*sizeof(inode_t), sizeof(inode_t), \
               BLOCK(bgpt[block_group].inode_table_address));
}

void update_or_bitmap(u_int32 address, u_int16 word, u_int8 mask)
{
  readLBA(address, 1, std_buf);
  /* writef("Addr: %u, word: %u, ", address, word); */
  std_buf[word] |= mask;
  writeLBA(address, 1, std_buf);
}

void flush_spb()
{
  /* writeLBA(2, 1, (u_int8*) spb); */ // FIXME
}

void flush_bgpt()
{
  writeLBA(BLOCK(1), block_factor * (bgpt_size / block_size), (u_int8*) bgpt);
}

u_int32 allocate_inode()
{
  bitset_t b;
  b.length = spb->inode_per_group;
  b.bits = (void*) std_buf;
  u_int32 ret = spb->first_free_inode;
  /* writef("FFI:%u, ", ret);  */
  u_int32 k = (ret-1) / spb->inode_per_group; // Current checked block
  u_int32 pos = (ret-1) % spb->inode_per_group;
  u_int32 address = bgpt[(ret-1) / spb->inode_per_group].inode_address_bitmap;
  readLBA(BLOCK(address), block_factor, std_buf);

  if(!spb->unalloc_inode_num) {
    return 0;
  }
  spb->unalloc_inode_num--;
  set_bit(b, pos, 1);
  writeLBA(BLOCK(address), block_factor, std_buf);
  bgpt[(ret-1) / spb->inode_per_group].unalloc_inode--;
  flush_bgpt();

  for(; k < 1 + (spb->inode_num - 1) / spb->inode_per_group; k++) {
    readLBA(BLOCK(bgpt[k].inode_address_bitmap), block_factor, std_buf);
    if(!bgpt[k].unalloc_inode) {
      pos = (u_int32) (-1);
      continue;
    }

    pos = param_ffb(b, pos + 1);
    if(pos != (u_int32) (-1)) {
      break;
    }
  }

  if(pos == (u_int32) (-1)) {
    return 0; // No free inode
  }

  spb->first_free_inode = k * spb->inode_per_group + pos + 1;
  flush_spb();
  return ret;
}

u_int8 unallocate_inode(u_int32 inode)
{
  if(inode < spb->first_free_inode) {
    spb->first_free_inode = inode;
  }
  u_int32 address = bgpt[(inode-1) / spb->inode_per_group].inode_address_bitmap;
  readLBA(BLOCK(address), block_factor, std_buf);
  if(! (((u_int16*)std_buf)[((inode-1)%2048) / 16] & (1<<((inode-1)%16))) ) {
    return 1; // Not allocated inode
  }
  spb->unalloc_inode_num++;
  bgpt[(inode-1) / spb->inode_per_group].unalloc_inode++;
  ((u_int16*)std_buf)[((inode-1)%2048) / 16] &= ~(1<<((inode-1)%16));
  writeLBA(BLOCK(address), block_factor, std_buf);
  flush_bgpt(); flush_spb();
  return 0;
}

u_int32 allocate_block(u_int32 prec)
{
  if(!prec || prec >= spb->block_num - 1) {
    prec = 1;
  }

  bitset_t b;
  b.length = spb->block_per_group;
  b.bits = (void*) std_buf;

  u_int32 k = prec / spb->block_per_group; // Current checked block
  u_int32 pos = prec % spb->block_per_group;
  u_int32 address = bgpt[prec / spb->block_per_group].block_address_bitmap;
  readLBA(BLOCK(address), block_factor, std_buf);

  if(!spb->unalloc_block_num) {
    return 0;
  }

  for(; k <= (spb->block_num - 1) / spb->block_per_group; k++) {
    readLBA(BLOCK(bgpt[k].block_address_bitmap), block_factor, std_buf);
    if(!bgpt[k].unalloc_block) {
      pos = (u_int32) (-1);
      continue;
    }

    pos = param_ffb(b, pos);
    if(pos != (u_int32) (-1)) {
      break;
    } else {
      pos++;
    }
  }

  if(pos != (u_int32) (-1)) {
    set_bit(b, pos, 1);
    writeLBA(BLOCK(bgpt[k].block_address_bitmap), block_factor, std_buf);
    spb->unalloc_block_num--;
    bgpt[k].unalloc_block--;
    flush_bgpt(); flush_spb();
    return k * spb->block_per_group + pos;
  }

  if(prec == 1) {
    throw("Superblock corrupted, no free block found");
  } else {
    writef("??");
    return allocate_block(1); // Restart from the beginning
  }
}

u_int8 unallocate_block(u_int32 block)
{
  bitset_t b;
  b.length = spb->block_per_group;
  b.bits = (void*) std_buf;

  u_int32 address = bgpt[block / spb->block_per_group].block_address_bitmap;
  readLBA(BLOCK(address), block_factor, std_buf);

  if(!get_bit(b, block % spb->block_per_group)) {
    return 1; // Not allocated block
  }
  set_bit(b, block % spb->block_per_group, 0);

  writeLBA(BLOCK(address), block_factor, std_buf);
  flush_bgpt(); flush_spb();
  writef("EN: %u, ", get_bit(b, block % spb->block_per_group));
  return 0;
}


/* The following function does not use std_buf if offset+length<12*block_size */
u_int32 read_inode_data(u_int32 inode, u_int8* buffer, u_int32 offset, \
                        u_int32 length)
{
  u_int32 width; // Length read, in bytes
  u_int32 to_read = offset / block_size;
  set_inode(inode, std_inode);
  u_int32 ofs = offset % block_size;

  if(length > block_size - ofs) {
    width = block_size - ofs;
  } else {
    width = length;
  }
  /* writef("Block to read: %u, with offset %u and width %u\n", to_read, ofs, width); */
  if(to_read < 12) {
    readPIO(BLOCK(std_inode->dbp[to_read]), ofs, width, buffer);
  } else {
    to_read -= 12;
    if(to_read < block_size / 4) {
      readPIO(BLOCK(std_inode->sibp), 0, block_size, std_buf);
      readPIO(BLOCK(((u_int32*)std_buf)[to_read]), ofs, width, buffer);
    } else {
      to_read -= block_size / 4;
      u_int32 dbsize = block_size*block_size;
      if(to_read < dbsize / 16) {
        readPIO(BLOCK(std_inode->dibp), 0, block_size, std_buf);
        readPIO(BLOCK(((u_int32*)std_buf)[to_read/block_size]), 0,  \
                block_size, std_buf);
        readPIO(BLOCK(((u_int32*)std_buf)[to_read % block_size]), \
                ofs, width, buffer);
      } else {
        to_read -= dbsize / 16;
        if(to_read >= dbsize*block_size / 64) {
          throw("Invalid offset");
        }
        readPIO(BLOCK(std_inode->tibp), 0, block_size, std_buf);
        readPIO(BLOCK(((u_int32*)std_buf)[to_read/dbsize]), 0, block_size, \
                std_buf);
        readPIO(BLOCK(((u_int32*)std_buf)[(to_read%dbsize)/block_size]), 0, \
                block_size, std_buf);
        readPIO(BLOCK(((u_int32*)std_buf)[to_read % block_size]), \
                ofs, width, buffer);
      }
    }
  }
  return width;
}

u_int32 write_inode_data(u_int32 inode, u_int8* buffer, u_int32 offset, \
                         u_int32 length)
{
  u_int32 width; // Length written, in bytes
  u_int32 to_write = offset / block_size;
  set_inode(inode, std_inode);
  u_int32 ofs = offset % block_size;

  if(length > block_size - ofs) {
    width = block_size - ofs;
  } else {
    width = length;
  }

  if(to_write < 12) {
    update_block(buffer, ofs, length, BLOCK(std_inode->dbp[to_write]));
  } else {
    u_int32 address;
    to_write -= 12;
    if(to_write < block_size / 4) {
      readPIO(BLOCK(std_inode->sibp), 0, block_size, std_buf);
      address = BLOCK(((u_int32*)std_buf)[to_write]);
      update_block(buffer, ofs, length, address);
    } else {
      to_write -= block_size / 4;
      u_int32 dbsize = block_size*block_size;
      if(to_write < dbsize / 16) {
        readPIO(BLOCK(std_inode->dibp), 0, block_size, std_buf);
        readPIO(BLOCK(((u_int32*)std_buf)[to_write/block_size]), 0,  \
                block_size, std_buf);
        address = BLOCK(((u_int32*)std_buf)[to_write % block_size]);
        update_block(buffer, ofs, length, address);
      } else {
        to_write -= dbsize / 16;
        if(to_write >= dbsize*block_size / 64) {
          throw("Invalid offset");
        }
        readPIO(BLOCK(std_inode->tibp), 0, block_size, std_buf);
        readPIO(BLOCK(((u_int32*)std_buf)[to_write/dbsize]), 0, block_size, \
                std_buf);
        readPIO(BLOCK(((u_int32*)std_buf)[(to_write%dbsize)/block_size]), 0, \
                block_size, std_buf);
        address = BLOCK(((u_int32*)std_buf)[to_write % block_size]);
        update_block(buffer, ofs, length, address);
      }
    }
  }
  return width;
}

u_int32 find_inode(string str_path, u_int32 root)
{
  list_t path = str_split(str_path, '/', TRUE);
  mem_free((void*) path->head); path = path->tail;

  u_int32 inode;
  if(str_path[0]=='/') {
    inode = 2;
  } else {
    inode = root;
  }
  dir_entry *entry = 0;
  int i; char* a; char* b;
  u_int32 endpos = (u_int32)std_buf + block_size;
  while(path->tail || ! (*((char*) path->head) == '\0') ) {
    read_inode_data(inode, std_buf, 0, block_size);
    entry = (void*) std_buf;
    while(entry->inode && (u_int32)entry < endpos) {
      a = (char*) path->head;
      b = (char*) &(entry->name);

      for(i=0; a[i] != '\0' && b[i] != '\0' && a[i]==b[i]; i++);
      // The difference with str_cmp is that b may not end with '\0'
      if(a[i] == b[i] || (a[i] == '\0' && i == entry->name_length)) {
        inode = entry->inode;
        break;
      }
      entry = (dir_entry*) (((u_int32)entry) + entry->size);
    }
    if((u_int32)entry == endpos) {
      while(path) {
        mem_free((void*) path->head);
        pop(&path);
      }
      return 0; // Not found
    }
    if(! path->tail) {
      mem_free((void*) path->head);
      pop(&path);
      return inode;
    }
    mem_free((void*) path->head);
    pop(&path);
  }
  mem_free((void*) path->head);
  pop(&path);
  return inode;
}


u_int16 set_dir_entry(dir_entry* e, u_int32 inode, u_int8 file_type, \
                      string name, u_int16 size)
{
  u_int8 len = (u_int8) str_length(name);
  u_int16 ret;
  e->inode = inode;
  e->name_length = len;
  if(size) {
    ret = 4 * (1 + (size-1)/4);
    e->size = ret;
  } else {
    ret = 4 * ( 1 + (len + sizeof(dir_entry) - 1)/4 );
    e->size = ret;
  }
  e->file_type = file_type;
  for(int i = 0; i < len; i++) {
    ((string) &(e->name))[i] = name[i];
  }

  return ret;
}

u_int8 add_file(u_int32 dir, u_int32 inode, u_int8 file_type, string name)
{
  if(!dir) {
    return 4;
  }
  if(!inode) {
    return 5;
  }
  u_int32 len = str_length(name);
  if(len>>8) {
    return 3;
  }
  read_inode_data(dir, std_buf, 0, block_size);
  dir_entry *entry = (void*) std_buf;
  u_int32 room = entry->size; // Occupied room until current entry (included)
  string b = (string) &(entry->name);
  u_int32 i;
  for(i=0; name[i] != '\0' && b[i] != '\0' && name[i]==b[i]; i++);
  if(name[i] == b[i] || (name[i] == '\0' && i == entry->name_length)) {
    return 1; // Already a file with the same name
  }

  while(entry->inode && room < block_size) { // Redundant condition (safer)
    entry = (dir_entry*) (((u_int32)entry) + entry->size);
    b = (string) &(entry->name);
    for(i=0; name[i] != '\0' && b[i] != '\0' && name[i]==b[i]; i++);
    if(name[i] == b[i] || (name[i] == '\0' && i == entry->name_length)) {
      return 1; // Already a file with the same name
    }
    room += entry->size;
  }
  
  u_int16 resize = 4 * ( 1 + (entry->name_length + sizeof(dir_entry) - 1)/4 );
  room = room - entry->size + resize;

  if(block_size - room < sizeof(dir_entry) + len) {
    // Shrinking each entry to its natural size (widened because of deletions)
    u_int32 endpos = (u_int32)std_buf + block_size;
    dir_entry* ref = (void*) ((u_int32)std_buf + 8);
    entry = ref;
    // 8 is the size (in bytes) of the first dir entry, ".", that was not erased

    while(entry->inode && (u_int32) entry < endpos) {
      room = entry->name_length;

      ref->name_length = room;
      ref->inode = entry->inode;
      ref->file_type = entry->file_type;
      ref->size = 4 * (1 + (room + sizeof(dir_entry) - 1)/4);

      for(i = 0; i < room; i++) { // Name copy
        ((u_int8*) ref)[8+i] = ((u_int8*) entry)[8+i];
      }
      entry = (dir_entry*) (((u_int32)entry) + entry->size);
      ref = (dir_entry*) (((u_int32)ref) + ref->size);
    }

    room = block_size - ((u_int32)ref - (u_int32)std_buf);

    if(room < sizeof(dir_entry) + len) {
      return 2; // No room for another entry in this directory
      /* This condition is sufficient because of the 4-bytes alignment */
    }
    set_dir_entry(ref, inode, file_type, name, room);
  }

  else {
    entry->size = resize;
    entry = (dir_entry*) (((u_int32)entry) + resize);
    set_dir_entry(entry, inode, file_type, name, block_size - room);
  }
  /* set_inode(dir, std_inode);*/ // std_inode already set by read_inode_data
  writeLBA(BLOCK(std_inode->dbp[0]), block_factor, std_buf);
  if(file_type & FILE_DIR) {
    std_inode->hard_links++;
    update_inode(dir, std_inode);
  }

  return 0;
}

// The next function only updates the parent directory, it does not unallocate
u_int8 remove_file(u_int32 dir, u_int32 inode)
{
  if(!dir) {
    return 4;
  }
  if(!inode) {
    return 5;
  }
  read_inode_data(dir, std_buf, 0, block_size);

  u_int32 endpos = (u_int32)std_buf + block_size;
  dir_entry *entry = (void*) std_buf; // "."
  dir_entry *prec = (void*) ((u_int32)entry + entry->size); // ".."
  entry = (dir_entry*) (((u_int32)prec) + prec->size);
  while((u_int32) entry < endpos) {
    if(entry->inode == inode) {
      break;
    }
    prec = (dir_entry*) (((u_int32)prec) + prec->size);
    entry = (dir_entry*) (((u_int32)entry) + entry->size);
  }

  if((u_int32)entry >= endpos) {
    return 1; // No such file found
  }

  prec->size += entry->size;
  writeLBA(BLOCK(std_inode->dbp[0]), block_factor, std_buf);
  if(entry->file_type & FILE_DIR) {
    std_inode->hard_links--;
    update_inode(dir, std_inode);
  }
  return 0;
}

void erase_file_data(u_int32 inode)
{
  set_inode(inode, std_inode);
  if(!std_inode->size_low) {
    return;
  }

  u_int32 blocks = 1 + (std_inode->size_low - 1) / block_size;
  std_inode->size_low = 0;
  update_inode(inode, std_inode);

  if(blocks <= 12) {
    for(u_int8 i = 0; i < blocks; i++) {
      unallocate_block(std_inode->dbp[i]);
    }
    return;
  }
  for(u_int8 i = 0; i < 12; i++) {
    unallocate_block(std_inode->dbp[i]);
  }

  readLBA(BLOCK(std_inode->sibp), block_factor, std_buf);
  blocks -= 12;
  u_int32 addr_per_block = block_size / 4;
  if(blocks <= addr_per_block) {
    for(u_int32 i = 0; i < blocks; i++) {
      unallocate_block(((u_int32*) std_buf)[i]);
    }
    unallocate_block(BLOCK(std_inode->sibp));
    return;
  }
  unallocate_block(BLOCK(std_inode->sibp));

  u_int32 curr_block;
  for(u_int32 i = 0; i < addr_per_block; i++) {
    unallocate_block(((u_int32*) std_buf)[i]);
  }
  blocks -= addr_per_block;
  for(u_int32 i = 0; i < addr_per_block; i++) {
    readLBA(BLOCK(std_inode->dibp), 4, std_buf);
    curr_block = BLOCK(((u_int32*) std_buf)[i]);
    readLBA(curr_block, 4, std_buf); 
    for(u_int32 j = 0; j < addr_per_block; j++) {
      unallocate_block(((u_int32*) std_buf)[j]);
      blocks--;
      if(!blocks) {
        unallocate_block(curr_block);
        unallocate_block(BLOCK(std_inode->dibp));
        return;
      }
    }
    unallocate_block(curr_block);
  }
  unallocate_block(BLOCK(std_inode->dibp));

  u_int32 curr_sq_block;
  for(u_int32 i = 0; i < addr_per_block; i++) {
    readLBA(BLOCK(std_inode->tibp), 4, std_buf);
    curr_sq_block = BLOCK(((u_int32*) std_buf)[i]);
    readLBA(curr_sq_block, 4, std_buf); 
    for(u_int32 j = 0; j < addr_per_block; j++) {
      curr_block = BLOCK(((u_int32*) std_buf)[j]);
      readLBA(curr_block, 4, std_buf); 
      for(u_int32 k = 0; k < addr_per_block; k++) {
        unallocate_block(((u_int32*) std_buf)[k]);
        blocks--;
        if(!blocks) {
          unallocate_block(curr_block);
          unallocate_block(curr_sq_block);
          unallocate_block(BLOCK(std_inode->tibp));
          return;
        }
      }
      unallocate_block(curr_block);
    }
    unallocate_block(curr_sq_block);
  }
  unallocate_block(BLOCK(std_inode->tibp));
}

u_int32 prepare_blocks(u_int32 inode, u_int32 used, u_int32 to_use)
{
  if(to_use <= used) {
    return 0;
  }
  u_int32 addr_per_block = block_size / 4;
  if(to_use > addr_per_block * addr_per_block * addr_per_block +    \
     addr_per_block * addr_per_block + addr_per_block + 12) {
    return 0;
  }
  u_int32 count = 0; // Total number of allocated blocks
  u_int32 addr = 1;

  if(used < 12) { //  DBP
    for(; used < 12; used++) {
      addr = allocate_block(addr);
      if(!addr) {
        update_inode(inode, std_inode);
        return count;
      }
      count++;
      std_inode->dbp[used] = addr;
      if(used == to_use) {
        update_inode(inode, std_inode);
        return count;
      }
    }
  }
  update_inode(inode, std_inode);

  used -= 12; to_use -=12;
  if(used < addr_per_block) { // SIBP

    if(!used) { // creation of inode.sibp
      addr = allocate_block(addr);
      if(!addr) {
        return count;
      }
      std_inode->sibp = addr;
    }
    update_inode(inode, std_inode);

    readLBA(BLOCK(std_inode->sibp), block_factor, std_buf);
    for(; used < addr_per_block; used++) {
      addr = allocate_block(addr);
      if(!addr) {
        writeLBA(BLOCK(std_inode->sibp), block_factor, std_buf);
        return count;
      }
      ((u_int32*) std_buf)[used] = addr;
      count++;
      if(used == to_use) {
        writeLBA(BLOCK(std_inode->sibp), block_factor, std_buf);
        return count;
      }
    }
    writeLBA(BLOCK(std_inode->sibp), block_factor, std_buf);
  }
  used -= addr_per_block; to_use -= addr_per_block;

  u_int32 curr_block;
  if(used < addr_per_block * addr_per_block) { // DIBP

    if(!used) { // Creation of inode.dibp
      addr = allocate_block(addr);
      if(!addr) {
        return count;
      }
      std_inode->dibp = addr;
    }
    update_inode(inode, std_inode);

    for(u_int32 i = used / addr_per_block; i < addr_per_block; i++) {
      readLBA(BLOCK(std_inode->dibp), 4, std_buf);
      if(!(used % addr_per_block)) { // New sub address block to create
        addr = allocate_block(addr);
        if(!addr) {
          return count;
        }
        ((u_int32*) std_buf)[i] = addr;
        writeLBA(BLOCK(std_inode->dibp), 4, std_buf);
      }
      curr_block = BLOCK(((u_int32*) std_buf)[i]);
      readLBA(curr_block, 4, std_buf);
      for(u_int32 j = used % addr_per_block; j < addr_per_block; j++) {
        addr = allocate_block(addr);
        if(!addr) {
          writeLBA(curr_block, 4, std_buf);
          return count;
        }
        ((u_int32*) std_buf)[j] = addr;
        used ++; count++;
        if(used == to_use) {
          writeLBA(curr_block, 4, std_buf);
          return count;
        }
      }
      writeLBA(curr_block, 4, std_buf);
    }
  }

  u_int32 sq_addr_per_block = addr_per_block * addr_per_block;
  u_int32 curr_sq_block;
  // TIBP
  used -= sq_addr_per_block; to_use -= sq_addr_per_block;

  if(!used) { // Creation of inode.tibp
    addr = allocate_block(addr);
    if(!addr) {
      update_inode(inode, std_inode);
      return count;
    }
    std_inode->tibp = addr;
  }
  update_inode(inode, std_inode);

  for(u_int32 i = used / sq_addr_per_block; i < addr_per_block; i++) {
    readLBA(BLOCK(std_inode->tibp), 4, std_buf);
    if(! (used % sq_addr_per_block)) {
      addr = allocate_block(addr);
      if(!addr) {
        return count;
      }
      ((u_int32*) std_buf)[i] = addr;
      writeLBA(BLOCK(std_inode->tibp), 4, std_buf);
    }
    curr_sq_block = BLOCK(((u_int32*) std_buf)[i]);
    readLBA(curr_sq_block, 4, std_buf); 
    for(u_int32 j = (used % sq_addr_per_block) / addr_per_block; \
        j < addr_per_block; j++) {
      if(! (used % addr_per_block)) {
        addr = allocate_block(addr);
        if(!addr) {
          return count;
        }
        ((u_int32*) std_buf)[j] = addr;
        writeLBA(curr_sq_block, 4, std_buf);
      }
      curr_block = BLOCK(((u_int32*) std_buf)[j]);
      readLBA(curr_block, 4, std_buf);
      for(u_int32 k = used % addr_per_block; k < addr_per_block; k++) {
        addr = allocate_block(addr);
        if(!addr) {
          writeLBA(curr_block, 4, std_buf);
          return count;
        }
        ((u_int32*) std_buf)[k] = addr;
        used++; count++;
        if(used == to_use) {
          writeLBA(curr_block, 4, std_buf);
          return count;
        }
      }
      writeLBA(curr_block, 4, std_buf);
    }
  }

  return count;
}

u_int8 delete_file(u_int32 dir, u_int32 inode)
{
  u_int8 error = remove_file(dir, inode);
  if(error) {
    return error; // If the inode is invalid, it should be detected now
  }
  erase_file_data(inode);
  error = unallocate_inode(inode);
  if(error) {
    throw("The file to delete was not allocated"); // Corrupted filesystem
  }
  return 0;
}

u_int32 create_file(u_int32 father, string name, u_int16 type, u_int8 ftype)
{
  if(!father) {
    return 0;
  }
  u_int32 num = allocate_inode();
  if(!num) {
    return 0;
  }
  u_int32 block = allocate_block(1);
  if(!block) {
    unallocate_inode(num);
    return 0;
  }
  u_int8 error = add_file(father, num, ftype, name);
  if(error) {
    unallocate_inode(num);
    unallocate_block(block);
    return 0;
  }
  u_int8 is_a_dir = !!(type & TYPE_DIR);
  std_inode->type = type;
  std_inode->hard_links = 2; // Father (+ itself)
  std_inode->sectors = block_factor;
  std_inode->dbp[0] = block;
  std_inode->size_low = block_size * is_a_dir;
  for(u_int8 i = 1; i < 12; i++) {
    std_inode->dbp[i] = 0;
  }
  std_inode->sibp = 0; std_inode->dibp = 0; std_inode->tibp = 0;
  update_inode(num, std_inode);

  if(is_a_dir) {
    u_int16 size = set_dir_entry((void*) std_buf, num, FILE_DIR, ".", 0);
    set_dir_entry((void*) ((u_int32) std_buf + size), father, FILE_DIR, "..",\
                  block_size - size);
    writeLBA(BLOCK(block), 1, std_buf);
  }

  return num;
}

u_int32 create_dir(u_int32 father, string name) {
  return create_file(father, name, TYPE_DIR | PERM_ALL, FILE_DIR);
}



void ls_dir(u_int32 inode)
{
  if(!inode) {
    writef("Invalid address\n");
    return;
  }
  /* writef("Reading data from inode %u\n", inode); */
  read_inode_data(inode, std_buf, 0, block_size);
  dir_entry *entry = (void*) std_buf;
  u_int32 endpos = (u_int32)std_buf + block_size;

  while(entry->size && (u_int32)entry < endpos) {
    writef("\n%c ", 195);
    for(u_int32 i = 0 ; i < entry->name_length ; i++) {
      writef("%c", ((u_int8*) &(entry->name))[i]);
    }
    set_inode(entry->inode, std_inode);
    writef("\t<-- inode = %u", entry->inode);
    entry = (dir_entry*) (((u_int32)entry) + entry->size);
  }
  writef("\n");
}



void filesystem_install()
{
  set_disk(FALSE);

  spb = (void*) mem_alloc(512); // Minimal size of a block
  readLBA(2, 1, (u_int8*) spb);
  if(spb->signature!=0xef53) {
    throw("Wrong superblock signature : is this ext2 ?");
  }

  block_size = 1024<<(spb->block_size);
  /* The next definition means: block_factor = block_size / 0x200*/
  block_factor = 2<<(spb->block_size);

  std_buf = mem_alloc(block_size);

  u_int32 block_group_num = (spb->block_num + spb->block_per_group - 1) / \
    spb->block_per_group;
  if(block_group_num != (spb->inode_num + spb->inode_per_group - 1) /   \
     spb->inode_per_group) {
    throw("Incoherent number of block groups");
  }

  bgpt_size = block_size * (1 + (block_group_num * sizeof(bgp_t) - 1)\
                                    / block_size);
  bgpt = (void*) mem_alloc(bgpt_size);
  readLBA(BLOCK(1), block_factor * (bgpt_size / block_size), (u_int8*) bgpt);
  /* bgpt[0].unalloc_block -= 2; // Blocks 0 and 1 are reserved. */
  /* spb->unalloc_block_num -= 2; */

  std_inode = mem_alloc(sizeof(inode_t));

  allocate_inode();
  u_int32 test1 =create_dir(2, "test1");
  u_int32 test2 = create_dir(2, "test2");
  /* u_int32 subtest =  */create_dir(test1, "subtest");
  u_int32 filetest = create_file(test2, "filetest", PERM_ALL | TYPE_FILE, FILE_REGULAR);

  add_file(test2, filetest, FILE_REGULAR, "filetest");

  kloug(100, "Filesystem installed\n");
}
