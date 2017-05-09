#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "ata_pio.h"
#include "malloc.h"
#include "string.h"

struct superblock {      // Starting byte
  u_int32 inode_num;             // 0
  u_int32 block_num;             // 4
  u_int32 reserved_num;          // 8
  u_int32 unalloc_block_num;     // 12
  u_int32 unalloc_inode_num;     // 16
  u_int32 this_block;            // 20
  u_int32 block_size;            // 24
  u_int32 fragment_size;         // 28
  u_int32 block_per_group;       // 32
  u_int32 fragment_per_group;    // 36
  u_int32 inode_per_group;       // 40
  u_int32 last_mount_time;       // 44
  u_int32 last_write_time;       // 48
  u_int16 mounts_since_fsck;     // 52
  u_int16 max_mounts_since_fsck; // 54
  u_int16 signature;             // 56
  u_int16 state;                 // 58
  u_int16 error_handling;        // 60
  u_int16 version_minor;         // 62
  u_int32 time_since_fsck;       // 64
  u_int32 max_time_since_fsck;   // 68
  u_int32 creator;               // 72
  u_int32 version_major;         // 76
  u_int16 allowed_user;          // 80
  u_int16 allowed_group;         // 82
  /* Afterward, extended superblock fiels (valid only if version_major > 0) */
  u_int32 first_free_inode;      // 84
  u_int16 inode_size;            // 88
  u_int16 this_block_group;      // 90
  u_int32 opt_features;          // 92
  u_int32 req_features;          // 96
  u_int32 req_for_write;         // 100
  u_int32 fs_id[4];              // 104
  char    name[16];              // 120
  char    mnt_path[64];          // 136
  u_int32 compression;           // 200
  u_int8  preallocate_for_files; // 204
  u_int8  preallocate_for_dirs;  // 205
  u_int16 just_boot;             // 206 - Specific to this implementation
  u_int32 journal_id[4];         // 208
  u_int32 journal_inode;         // 224
  u_int32 journal_device;        // 228
  u_int32 orphan_inode_list_head;// 232
  /* Unused */                   // 236 - 1023
}__attribute__((packed));

typedef struct superblock superblock_t;

#define STATE_CLEAN 0x0001
#define STATE_ERROR 0x0002

#define ERROR_IGNORE  0x0001
#define ERROR_REMOUNT 0X0002
#define ERROR_PANIC   0X0003

struct block_group_descriptor {
  u_int32 block_address_bitmap;
  u_int32 inode_address_bitmap;
  u_int32 inode_table_address;
  u_int16 unalloc_block;
  u_int16 unalloc_inode;
  u_int16 dir_num;
  u_int16 unused[7];
}__attribute__((packed));

typedef struct block_group_descriptor bgp_t;

struct inode_t {
  u_int16 type;
  u_int16 user_id;
  u_int32 size_low;
  u_int32 last_access_time;
  u_int32 creation_time;
  u_int32 last_modif_time;
  u_int32 deletion_time;
  u_int16 group_id;
  u_int16 hard_links;
  u_int32 sectors;
  u_int32 flags;
  u_int32 OS_spec1;
  u_int32 dbp[12]; // Direct Block Pointer
  u_int32 sibp; // Single Indirect Block Pointer
  u_int32 dibp; // Doubly Indirect Block Pointer
  u_int32 tibp; // Triply Indirect Block Pointer
  u_int32 generation;
  u_int32 reserved[2];
  u_int32 fragment_block_address;
  u_int32 OS_spec2[3];
}__attribute__((packed));

typedef struct inode_t inode_t;


/* Masks for the type field */
#define TYPE_FIFO      0x1000
#define TYPE_CHAR_DEV  0x2000
#define TYPE_DIR       0x4000
#define TYPE_BLOCK_DEV 0x6000
#define TYPE_FILE      0x8000
#define TYPE_SYMB_LINK 0xA000
#define TYPE_UNIX_SOCK 0xC000

#define PERM_OTHER_E  0x001
#define PERM_OTHER_W  0x002
#define PERM_OTHER_R  0x004
#define PERM_GROUP_E  0x008
#define PERM_GROUP_W  0x010
#define PERM_GROUP_R  0X020
#define PERM_USER_E   0x040
#define PERM_USER_W   0x080
#define PERM_USER_R   0x100
#define PERM_STICKY   0x200
#define PERM_GROUP_ID 0x400
#define PERM_USER_ID  0x800
#define PERM_ALL      0x1FF
/* inode_t->type is TYPE | PERM */

struct dir_entry {
  u_int32 inode;
  u_int16 size;
  u_int8  name_length;
  u_int8  file_type;
  u_int8  name; // Not actually 8 bits long, this is just an entry point
}__attribute__((packed));

typedef struct dir_entry dir_entry;

#define FILE_UNKNOWN   0x0
#define FILE_REGULAR   0x1
#define FILE_DIR       0x2
#define FILE_CHAR_DEV  0x3
#define FILE_BLOCK_DEV 0x4
#define FILE_FIFO      0x5
#define FILE_SOCKET    0x6
#define FILE_SYMB_LINK 0x7


/**
 *  @name find_inode - Finds the inode corresponding to an inode number
 *  @param inode     - The inode number
 *  @param buffer    - The output buffer of the inode, must be at least 128B wide
 */
void find_inode(u_int32 inode, inode_t *buffer);

/**
 *  @name allocate_inode - Allocates a new inode
 *  @return ret          - The inode number of the new used inode. 
 *                         0 if there is no free inode available
 */
u_int32 allocate_inode();

/**
 *  @name unallocate_inode - Unallocates an inode
 *  @param inode - The inode to free
 *  @return      - 0: No error
 *                 1: The inode was not allocated before. No change was made
 */
u_int8 unallocate_inode(u_int32 inode);

/**
 *  @name allocate_block - Allocates a new block
 *  @return ret          - The block number of the new used block. 
 *                         0 if there is no free block available
 */
u_int32 allocate_block(u_int32 prec);

/**
 *  @name unallocate_block - Unallocates a block
 *  @param inode - The block to free
 *  @return      - 0: No error
 *                 1: The block was not allocated before. No change was made
 */
u_int8 unallocate_block(u_int32 block);

/**
 *  @name read_inode_data - Copies a certain amount of data from the disk to
 *  a given buffer. Only reads up to the end of the current block.
 *
 *  @param inode  - The inode number of the file to read
 *  @param buffer - The output buffer
 *  @param offset - Offset within the file, in words (NOT in bytes)
 *  @param length - The length of the data to copy, in words (NOT in bytes)
 *
 *  @return       - The actually read length of data
 */
u_int32 read_inode_data(u_int32 inode, u_int16* buffer, u_int32 offset, \
                        u_int32 length);

/**
 *  @name write_inode_data - Writes a certain amount of data to the disk from
 *  a given buffer. Only writes up to the end of the current block.
 *
 *  @param inode  - The inode number of the file to write
 *  @param buffer - The input buffer
 *  @param offset - Offset within the file, in words (NOT in bytes)
 *  @param length - The length of the data to copy, in words (NOT in bytes)
 *
 *  @return       - The actually written length of data
 */
u_int32 write_inode_data(u_int32 inode, u_int16* buffer, u_int32 offset, \
                         u_int32 length);


/**
 *  @name open_file - Opens a file
 *  @param str_path - The path to the file
 *  @return inode   - The inode number of the file
 */
u_int32 open_file(string str_path);

/**
 *  @name add_file - Adds a file to a directory
 *
 *  @param dir       - The inode number of the directory
 *  @param inode     - The inode number of the file to add
 *  @param file_type - The file type for the directory entry (FILE_...)
 *  @param name      - A string containing the name of the new file
 *
 *  @return          - 0: No error
 *                     1: Already a file with the same name in the directory
 *                     2: No room for another file in this directory
 */
u_int8 add_file(u_int32 dir, u_int32 inode, u_int8 file_type, string name);

/**
 *  @name remove_file - Removes the file from the directory
 *  WARNING: Does not erase the file itself, only removes it from search
 *
 *  @param dir        - The parent directory
 *  @param inode      - The inode number of the file to remove
 *
 *  @param return     - 0: No error
 *                      1: No such file found in the directory
 */
u_int8 remove_file(u_int32 dir, u_int32 inode);


/**
 *  @name create_dir - Creates a directory
 *  @param father    - The inode number of the father directory of this one
 *  @param name      - The name of the directory
 *  @return num      - The inode number of the created directory
 */
u_int32 create_dir(u_int32 father, string name);

/**
 *  @name  ls_dir - Prints to the screen the contents of a directory
 *  @param inode  - The inode number of the directory
 */
void ls_dir(u_int32 inode);

/**
 *  @name filesystem_install - initializes all the necessary constants of the fs
 */
void filesystem_install();

#endif
