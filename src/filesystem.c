#include "filesystem.h"
#include "malloc.h"
#include "kheap.h"


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
/* The above standard buffer is set up with size 512 using mem_alloc.
 * Its purpose is to give access to memory within a function, without a call
 * to mem_alloc each time such a buffer is needed.
 * Warning: its content may be modified by any of the following elementary
 * functions, so do not use it while calling such functions. */

#define BLOCK(n) (block_factor*(n))

void find_inode(u_int32 inode, inode_t *buffer)
{
  u_int32 block_group = (inode-1) / spb->inode_per_group;
  u_int32 index = (inode-1) % spb->inode_per_group;
  readPIO(BLOCK(bgpt[block_group].inode_table_address), \
          index*sizeof(inode_t)/2, sizeof(inode_t)/2, (u_int16*) buffer);
}

u_int8 allocate_inode(inode_t *ret)
{
  if(!(spb->unalloc_inode_num & spb->first_free_inode)) { return 1; }
  find_inode(spb->first_free_inode, ret);
  spb->unalloc_inode_num--;

  u_int32 k = spb->first_free_inode>>11; // Current checked sector
  u_int8 i = (u_int8) (spb->first_free_inode % 2048)>>4; // Current checked word
  u_int8 j = (u_int8) spb->first_free_inode % 16; // Current checked bit
  u_int32 address=bgpt[(int) (k<<11)/spb->inode_per_group].inode_address_bitmap;
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
          return 0;
        }
        j++;
      }
    }
    j = 0; i++;
  }
  i = 0; k++;
  while(k < (spb->inode_num>>11)) {
    address = bgpt[(int) (k<<11) / spb->inode_per_group].inode_address_bitmap;
    readPIO(BLOCK(address), k<<8, 256, std_buf);
    while(i < 128) {
      if(std_buf[i]!=0xff) {
        while(j < 16) {
          if(!(std_buf[i] & (1<<j))) {
            spb->first_free_inode = (k<<11) + (i<<4) + j;
            return 0;
          }
          j++;
        }
      }
      j = 0; i++;
    }
    i = 0; k++;
  }
  spb->first_free_inode = 0; // No next free inode
  return 0;
}

void unallocate_inode(u_int32 inode)
{
  spb->unalloc_inode_num++;
  if(inode < spb->first_free_inode) {
    spb->first_free_inode = inode;
  }
  readLBA(BLOCK(bgpt[(inode-1) / spb->inode_per_group].inode_address_bitmap), \
          1, std_buf);
}

void parse_superblock()
{
  std_buf = mem_alloc(512);
  
  spb = (void*) mem_alloc(sizeof(superblock_t));
  readPIO(2, 0, sizeof(superblock_t)/2, (u_int16*) spb);
  if(spb->signature!=0xef53) {
    throw("Wrong superblock signature : is this ext2 ?");
  }
  
  block_factor = 1 << (spb->block_size+1);
  /* The original formula is block_factor = true_block_size / 0x200, where
   * true_block_size = 1<<(10+block_size) */

  /* u_int32 block_size = 1024<<(spb->block_size); */
  u_int32 block_group_num = (spb->block_num + spb->block_per_group - 1) / spb->block_per_group;
  if(block_group_num != (spb->inode_num + spb->inode_per_group - 1) / spb->inode_per_group) {
    throw("Incoherent number of block groups");
  }

  u_int32 bgpt_size = block_group_num * sizeof(bgp_t);
  bgpt = (void*) mem_alloc(bgpt_size);
  readPIO(BLOCK(1), 0, bgpt_size, (u_int16*) bgpt);
  for(int i=0; i<2; i++) {
    writef("block bitmap: %x,inode bitmap: %x,inode table: %x,\nunalloc block: %x, unalloc inode: %x, dir_num: %x\n", bgpt[i].block_address_bitmap, bgpt[i].inode_address_bitmap, bgpt[i].inode_table_address, bgpt[i].unalloc_block, bgpt[i].unalloc_inode, bgpt[i].dir_num);
  }

  inode_t *curr_inode = mem_alloc(128);

  writef("\nInitial first free inode: %u\n",spb->first_free_inode);
  allocate_inode(curr_inode);
  writef("Current first free inode: %u\n",spb->first_free_inode);
  allocate_inode(curr_inode);
  writef("Current first free inode: %u\n",spb->first_free_inode);

  for(int i = 0; i < 256; i++) {
    std_buf[i] = i;
  }
  writeLBA(BLOCK(20),1,std_buf);
  for(int i = 0; i < 256; i++) {
    std_buf[i] = 0x100 | i;
  }
  writeLBA(BLOCK(20)+1,1,std_buf);
  
  for(int i = 0; i < 256; i++) {
    std_buf[i] = 0;
  }
  writef("\n");
  readPIO(BLOCK(20),0,256,std_buf);
  writef("\n%u,%u,%u ; %x\n", std_buf[0],std_buf[1],std_buf[2], std_buf[255]);
  readPIO(BLOCK(20),1,256,std_buf);
  writef("\n%u,%u,%u ; %x\n", std_buf[0],std_buf[1],std_buf[2], std_buf[255]);
  readPIO(BLOCK(20),0,256,std_buf);
  writef("\n%u,%u,%u ; %x\n", std_buf[0],std_buf[1],std_buf[2], std_buf[255]);
  
  /* writef("\n"); */
  /* inode_t *inode_ex = mem_alloc(2*sizeof(inode_t)); */
  /* find_inode(3, inode_ex); */
  /* readPIO(BLOCK(bgpt[1].inode_table_address),0,sizeof(inode_t),(u_int16*)&inode_ex[1]); */
  /* for(int k = 0; k < 2; k++) { */
  /*   for(int i = 64; i < 128; i++) { */
  /*     writef("%x, ", ((u_int16*)&inode_ex[k])[i]); */
  /*   } */
  /*   writef("\ntype :%x, blocks: %u\n\n",inode_ex[k].type, inode_ex[k].blocks); */
  /* } */
  /* u_int16 buffer[128]; */
  /* readPIO(BLOCK(0x23),256,128,buffer); */
  /* for(int i = 64; i < 128; i++) { */
  /*   writef("%x, ", buffer[i]); */
  /* } */
}
