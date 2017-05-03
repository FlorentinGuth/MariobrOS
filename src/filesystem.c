#include "filesystem.h"
#include "kheap.h"

superblock_t *spb = 0; // Superblock address

/* lba n means address n * 0x200
 * Each sector is 2048 = 0x800 addresses wide
 * Thus, sector 2 starts at 0x800 * 0x200 = 0x100000 = lba 0x800
 * The spb is at lba 2 of sector 2, so 0x803 is the first block
 */
#define BLOCK(n) (0x803+(n))

void set_spb()
{
  spb = (void*) kmalloc(sizeof(superblock_t));
  readPIO(BLOCK(-1), 1, (u_int16*) spb);
  if(spb->signature!=0xef53) {
    throw("Wrong Spb signature : is this ext2 ?");
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
  writef("inode number: %u \nblock number: %u \nblock size: %u \nblock per group: %u \nstate: %u\nversion_major=%x", spb->inode_num, spb->block_num, 1024<<(spb->block_size), spb->block_per_group, spb->state, spb->version_major);
  /* u_int32 block_size = 1024<<(spb->block_size); */
  u_int16 buffer[256];
  writef("\n");
  if(spb->block_size) { // block_size = 1024
    readPIO(BLOCK(1),1,buffer);
  } else {
    readPIO(BLOCK(2),1,buffer);
  }
  for(int i = 0; i<256; i++) {
    writef("%x, ", buffer[i]);
  }
}
