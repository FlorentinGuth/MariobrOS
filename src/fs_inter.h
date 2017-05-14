#ifndef FS_INTER_H
#define FS_INTER_H

#include "filesystem.h"

/* Open flags */
#define O_RDONLY   0x01
#define O_WRONLY   0x02
#define O_RDWR     0x03 // Same as O_RDONLY | O_WRONLY
#define O_APPEND   0x04
#define O_CREAT    0x08
#define O_TRUNC    0x10
#define O_EXCL     0x20

/* Seek commands */
#define SEEK_SET   0x1
#define SEEK_CUR   0x2
#define SEEK_END   0x4 

typedef u_int32* fd;

struct file_description {
  u_int32 inode;
  u_int32 pos;
  u_int32 size;
  u_int16 mode;
  fd      this; // Its own file descriptor
} __attribute__((packed));

typedef struct file_description fdt_e;


typedef struct stats {
  u_int32 st_ino;
  u_int8 st_kind;
  u_int16 st_perm;
  u_int16 st_nlink;
  u_int32 st_size;
} stats;

/**
 *  @name openfile - Opens a file given flags and permissions
 *
 *  @param path    - The path to the file
 *  @param oflag   - The open flags
 *  @param perm    - The file access rights
 *
 *  @return        - The file descriptor for the given file or 0 if an error
 *                   occured
 */
fd openfile(string path, u_int8 oflag, u_int16 fperm);

/**
 *  @name openker - Opens a file in read/write with maximum rights
 *  @param path   - The path to the file
 *  @return       - Same as openfile
 */
fd openker(string path);
  
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
 *  @name fstat - Sets stats of a file
 *  @param f    - The file descriptor
 *  @param s    - A pointer to a stats structure
 */
void fstat(fd f, stats* s);

/**
 *  @name lseek   - Set the current position for a file descriptor
 *
 *  @param f      - The file descriptor
 *  @param offset - The new offset (according to the seek command)
 *  @param seek   - The seek command, SEEK_SET, SEEK_CUR or SEEK_END
 *
 *  @return       - The new actual offset from the beginning of the file
 */
u_int32 lseek(fd f, s_int32 offset, u_int8 seek);


/**
 *  @name close - Closes a file descriptor, ensuring it is no more valid
 *  @param f    - The file descriptor to close
 */
void close(fd f);


/**
 *  @name fs_inter_install - Installs the filesystem interface
 */
void fs_inter_install();

#endif
