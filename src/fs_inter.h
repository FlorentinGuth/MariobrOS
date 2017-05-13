#ifndef FS_INTER_H
#define FS_INTER_H

#include "filesystem.h"

#define O_RDONLY   0x01
#define O_WRONLY   0x02
#define O_RDWR     0x03 // Same as O_RDONLY | O_WRONLY
#define O_APPEND   0x04
#define O_CREAT    0x08
#define O_TRUNC    0x10
#define O_EXCL     0x20

typedef enum open_flag open_flag;

struct file_description {
  u_int32 inode;
  u_int32 pos;
  u_int32 size;
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
fd openfile(string path, u_int8 oflag, u_int16 fperm);

/**
 *  @name read    - Reads from a file
 *
 *  @param f      - The file descriptor
 *  @param buffer - The output buffer
 *  @param offset - The starting offset in the output buffer
 *  @param length - The number of bytes to read
 *
 *  @return       - The number of bytes actually read or
 *                  -1: Invalid file descriptor
 *                  -2: File is not opened with the read flag
 */
u_int32 read(fd f, u_int8* buffer, u_int32 offset, u_int32 length);

/**
 *  @name read    - Writes to a file
 *
 *  @param f      - The file descriptor
 *  @param buffer - The input buffer
 *  @param offset - The starting offset in the input buffer
 *  @param length - The number of bytes to write
 *
 *  @return       - The number of bytes actually written or
 *                  -1: Invalid file descriptor
 *                  -2: File is not opened with the written flag
 *                  -3: The block allocation failed
 */
u_int32 write(fd f, u_int8* buffer, u_int32 offset, u_int32 length);

/**
 *  @name close - Closes a file descriptor
 *  @param f    - The file descriptor to close
 *  WARNING: a closed file descriptor may refer to another fd...
 */
void close(fd f);

/**
 *  @name fs_inter_install - Installs the filesystem interface
 */
void fs_inter_install();

#endif
