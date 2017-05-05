#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "ata_pio.h"

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
  u_int16 unused;                // 206
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
  u_int32 disk_sectors_used;
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
#define TYPE_FIFO      0x1000;
#define TYPE_CHAR_DEV  0x2000;
#define TYPE_DIR       0x4000;
#define TYPE_BLOCK_DEV 0x6000;
#define TYPE_FILE      0x8000;
#define TYPE_SYMB_LINK 0xA000;
#define TYPE_UNIX_SOCK 0xC000;

#define PERM_OTHER_E  0x001;
#define PERM_OTHER_W  0x002;
#define PERM_OTHER_R  0x004;
#define PERM_GROUP_E  0x008;
#define PERM_GROUP_W  0x010;
#define PERM_GROUP_R  0X020;
#define PERM_USER_E   0x040;
#define PERM_USER_W   0x080;
#define PERM_USER_R   0x100;
#define PERM_STICKY   0x200;
#define PERM_GROUP_ID 0x400;
#define PERM_USER_ID  0x800;
/* inode_t->type is TYPE | PERM */

void analyze_superblock();
  
#endif
