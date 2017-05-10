#ifndef FS_INTER_H
#define FS_INTER_H

#include "filesystem.h"

enum open_flag {
  O_RDONLY, O_WRONLY, O_RDWR, O_APPEND, O_CREAT, O_TRUNC, O_EXCL
};

typedef enum open_flag open_flag;

struct file_description {
  u_int32 inode;
  u_int32 pos;
  u_int16 mode;
} __attribute__((packed));

typedef struct file_description fdt_e;


typedef s_int32 fd;

/**
 *  @name openfile - Opens a file given flags and permissions
 *
 *  @param path    - The path to the file
 *  @param oflag   - The open flags
 *  @param perm    - The file access rights
 *
 *  @return        - The file descriptor for the given file
 */
fd openfile(string path, open_flag oflag, u_int16 fperm);

/**
 *  @name fs_inter_install - Installs the filesystem interface
 */
void fs_inter_install();

#endif
