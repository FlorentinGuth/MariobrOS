#include "filesystem.h"
#include "malloc.h"
#include "kheap.h"

superblock_t *spb = 0; // Superblock address
bgp_t *bgpt = 0; // Block group descriptor table


#define BLOCK(n) (4*(n))

void set_spb()
{
  spb = (void*) mem_alloc(sizeof(superblock_t));
  /* lba n means address n * 0x200
   * Each sector is 2048 = 0x800 addresses wide
   * Thus, sector 2 starts at 0x800 * 0x200 = 0x100000 = lba 0x800
   * The spb is at lba 2 of sector 2, so 0x802 is the first block
   */
  readPIO(2, 0, sizeof(superblock_t), (u_int16*) spb);
  if(spb->signature!=0xef53) {
    throw("Wrong superblock signature : is this ext2 ?");
  }
}

u_int32 group_of_inode(u_int32 inode)
{
  return (inode-1) / spb->inode_per_group;
}

u_int32 index_of_inode(u_int32 inode)
{
  return (inode-1) % spb->inode_per_group;
}

void analyze_superblock()
{  
  set_spb();
  writef("inode number: %u, block number: %u, block size: %u, \nblock per group: %u, state: %u, version_major=%x, inode per group : %x\n\n", spb->inode_num, spb->block_num, 1024<<(spb->block_size), spb->block_per_group, spb->state, spb->version_major, spb->inode_per_group);
  /* u_int32 block_size = 1024<<(spb->block_size); */
  u_int32 block_group_num = (spb->block_num + spb->block_per_group - 1) / spb->block_per_group;
  if(block_group_num != (spb->inode_num + spb->inode_per_group - 1) / spb->inode_per_group) {
    throw("Incoherent number of block groups");
  }
  u_int32 bgpt_size = block_group_num * sizeof(bgp_t);
  writef("bgpt_size: %u\n", bgpt_size);
  bgpt = (void*) mem_alloc(bgpt_size);
  // 4 if block_size = 1024, 3 else:
  readPIO(BLOCK(1), 0, bgpt_size, (u_int16*) bgpt);
  for(int i=0; i<2; i++) {
    writef("block bitmap: %x,inode bitmap: %x,inode table: %x,\nunalloc block: %x, unalloc inode: %x, dir_num: %x\n", bgpt[i].block_address_bitmap, bgpt[i].inode_address_bitmap, bgpt[i].inode_table_address, bgpt[i].unalloc_block, bgpt[i].unalloc_inode, bgpt[i].dir_num);
  }

  inode_t inode_table[6];
  readPIO(4*((u_int32)bgpt[1].inode_table_address),0,sizeof(inode_table),(u_int16*)inode_table);

  writef("\n\n");
  for(int i=0; i<6; i++) {
    writef("type: %x, size: %u, group_id: %u, hard links: %u\n", inode_table[i].type,inode_table[i].size_low,inode_table[i].group_id,inode_table[i].hard_links);
  }

}
