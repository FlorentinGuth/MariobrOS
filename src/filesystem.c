#include "filesystem.h"

/* lba n means sector n, with sector_size = 0x200 = 512 bytes. The volume starts
 * at 1M = 0x10000 in memory. Thus, lba n is the address 512 * n from the 
 * beginning  of the volume, that is at the physical address 0x10000 + n * 0x200.
 * The superpblock is at lba 2, so 0x10400 is the position of block 0.
 * Each block is 2048 = 256 * 8 bytes wide.
 */


superblock_t *spb = 0; // Superblock address
bgp_t *bgpt = 0; // Block group descriptor table
u_int32 block_factor = 0; // Only used in the definition of the BLOCK macro
u_int16 *std_buf = 0;
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

void find_inode(u_int32 inode, inode_t *buffer)
{
  u_int32 block_group = (inode-1) / spb->inode_per_group;
  u_int32 index = (inode-1) % spb->inode_per_group;
  readPIO(BLOCK(bgpt[block_group].inode_table_address), \
          index*sizeof(inode_t)/2, sizeof(inode_t)/2, (u_int16*) buffer);
}

u_int32 allocate_inode()
{
  if(!(spb->unalloc_inode_num && spb->first_free_inode)) {
    return 0;
  }
  u_int32 ret = spb->first_free_inode;
  spb->unalloc_inode_num--;

  u_int32 k = spb->first_free_inode>>11; // Current checked sector
  u_int8 i = (u_int8) (spb->first_free_inode % 2048)>>4; // Current checked word
  u_int8 j = (u_int8) spb->first_free_inode % 16; // Current checked bit

  u_int32 address=bgpt[(k<<11)/spb->inode_per_group].inode_address_bitmap;
  readLBA(BLOCK(address)+k/2, 1, std_buf);
  std_buf[i+((k%2)<<8)] |= (1<<j);
  writeLBA(BLOCK(address)+k/2, 1, std_buf); // Updating the inode bitmap
  j++;
  bgpt[(int) (k<<11)/spb->inode_per_group].unalloc_inode--;

  while(i < 128) { // The first loop is inlined so as not to re-read the bitmap
    if(std_buf[i]!=0xff) {
      while(j < 16) {
        if(!(std_buf[i] & (1<<j))) {
          spb->first_free_inode = (k<<11) + (i<<4) + j;
          return ret;
        }
        j++;
      }
    }
    j = 0; i++;
  }
  i = 0; k++;

  while(k < (spb->inode_num>>11)) {
    address = bgpt[(k<<11) / spb->inode_per_group].inode_address_bitmap;
    readPIO(BLOCK(address), k<<8, 256, std_buf);
    while(i < 128) {
      if(std_buf[i]!=0xff) {
        while(j < 16) {
          if(!(std_buf[i] & (1<<j))) {
            spb->first_free_inode = (k<<11) + (i<<4) + j;
            return ret;
          }
          j++;
        }
      }
      j = 0; i++;
    }
    i = 0; k++;
  }

  spb->first_free_inode = 0; // No next free inode
  return ret;
}

u_int8 unallocate_inode(u_int32 inode)
{
  if(inode < spb->first_free_inode) {
    spb->first_free_inode = inode;
  }
  u_int32 address = bgpt[(inode-1) / spb->inode_per_group].inode_address_bitmap;
  readLBA(BLOCK(address), 1, std_buf);
  if(! (std_buf[(inode%2048) / 16] & (1<<(inode%16))) ) {
    return 1; // Not allocated inode
  }
  spb->unalloc_inode_num++;
  bgpt[(inode-1) / spb->inode_per_group].unalloc_inode++;
  std_buf[(inode%2048) / 16] &= ~(1<<(inode%16));
  writeLBA(BLOCK(address), 1, std_buf);
  return 0;
}

/* The following function does not use std_buf */
u_int32 read_inode_data(u_int32 inode, u_int16* buffer, u_int32 offset, \
                        u_int32 length)
{
  u_int32 block_size = 512<<(spb->block_size); // Block size in words
  find_inode(inode, std_inode);
  u_int32 copied = offset;
  length += offset;
  u_int8 position; // The data block to read
  while(copied < length) {
    position = copied / block_size;
    if(position<12) { // Direct Blocks
      if(length - copied >= block_size) {
        readPIO(BLOCK(std_inode->dbp[position]), 0, block_size/2, \
                &buffer[copied]);
        copied += block_size;
      } else {
        readPIO(BLOCK(std_inode->dbp[position]), 0, length-copied, \
                &buffer[copied]);
        copied = length;
      }
    } else {
      throw("Not yet implemented"); // TODO ibp !
    }
  }
  return copied - offset;
}


u_int32 open_file(string str_path)
{
  list_t path = str_split(str_path, '/', TRUE);
  u_int32 inode = 2; // root directory
  dir_entry *entry = 0;
  int i; char* a; char* b;

  while(path->head) {
    read_inode_data(inode, std_buf, 0, 256); // TODO the entire block
    entry = (void*) std_buf;
    while(entry->size) {
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
    if(!path->tail)
      return inode;
    path = path->tail;
  }

  return inode;
}

void ls_dir(u_int32 inode)
{
  read_inode_data(inode, std_buf, 0, 256);
  dir_entry *entry = (void*) std_buf;

  while(entry->size) {
    writef("\n-->  ");
    for(u_int8 i = 0 ; i < entry->name_length ; i++) {
      writef("%c", ((u_int8*) &(entry->name))[i]);
    }
    writef("  <-- inode = %u", entry->inode);
    entry = (dir_entry*) (((u_int32)entry) + entry->size);
  }
  writef("\n  _____\n\n");
}


void filesystem_install()
{
  set_disk(FALSE);

  /* u_int32 block_size = 1024<<(spb->block_size); */
  std_buf = mem_alloc(512);

  spb = (void*) mem_alloc(sizeof(superblock_t));
  readPIO(2, 0, sizeof(superblock_t)/2, (u_int16*) spb);
  if(spb->signature!=0xef53) {
    throw("Wrong superblock signature : is this ext2 ?");
  }

  /* The next definition is a reformulation of:
   * block_factor = true_block_size / 0x200, where 
   * true_block_size = 1<<(10 + spb->block_size) */
  block_factor = 1 << (spb->block_size+1);

  u_int32 block_group_num = (spb->block_num + spb->block_per_group - 1) / \
    spb->block_per_group;
  if(block_group_num != (spb->inode_num + spb->inode_per_group - 1) /   \
     spb->inode_per_group) {
    throw("Incoherent number of block groups");
  }

  u_int32 bgpt_size = block_group_num * sizeof(bgp_t);
  bgpt = (void*) mem_alloc(bgpt_size);
  readPIO(BLOCK(1), 0, bgpt_size/2, (u_int16*) bgpt);

  std_inode = mem_alloc(sizeof(inode_t));

  inode_t *root_inode = mem_alloc(sizeof(inode_t));
  find_inode(2, root_inode);

  /* ls_dir(open_file("/../boot/./grub/../../boot/grub/locale/.././fonts/..")); */
}
