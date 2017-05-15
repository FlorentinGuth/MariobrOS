#ifndef LIB_H
#define LIB_H


#define NULL  0

typedef unsigned int   u_int32;
typedef   signed int   s_int32;
typedef unsigned short u_int16;
typedef   signed short s_int16;
typedef unsigned char  u_int8;
typedef   signed char  s_int8;

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

/* Permission flags */
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


typedef u_int32* fd;

typedef unsigned char bool;
#define FALSE 0
#define TRUE  1


typedef char* string;


void *syscall_malloc(u_int32 size);

void syscall_free(void *ptr);

/**
 *  @name syscall_open - Opens a file
 *
 *  @param path        - The path to the file
 *  @param oflag       - The open flags
 *  @param perm        - The file access rights
 *
 *  @return            - The file descriptor for the given file or 0 if an error
 *  occured
 */
fd syscall_open(string path, u_int8 oflag, u_int16 fperm);

/**
 *  @name syscall_close - Closes a file descriptor, ensuring it is no more valid
 *  @param f            - The file descriptor to close
 */
void syscall_close(fd f);

/**
 *  @name syscall_read    - Reads from a file
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
u_int32 syscall_read(fd f, u_int8* buffer, u_int32 offset, u_int32 length);

/**
 *  @name write   - Writes to a file
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
u_int32 syscall_write(fd f, u_int8* buffer, u_int32 offset, u_int32 length);

/**
 *  @name syscall_fork - Creates a new process with a new, copied context
 *  @param priority    - The priority to give to the child process
 *  @param pid         - A pointer toward an integer that will be set with a process id
 *  @return u_int32    - 0 if the fork failed (pid has not been modified)
 *                       1 for the parent process, pid contains the id of the child process
 *                       2 for the child process, pid contains the id of the parent process
 */
u_int32 syscall_fork(u_int32 priority, u_int32 *pid);

/**
 *  @name syscall_exit  - Terminates the process
 *  The child of the process have their parent replaced by the init process, while
 *  the parent of the process will be given (once he calls wait) the return value.
 *  @param return_value - The return value of the exiting process
 *  @return void
 */
void syscall_exit(u_int32 return_value);

/**
 *  @name syscall_wait  - Waits for a child process to call exit
 *  @param pid          - Will contain the pid of the exited child process
 *  @param return_value - Will contain the return value of the exited child process
 *  @return bool        - 0 if the process has no children (pid and return_value won't be modified)
 *                        1 if the call succeeded, and pid and return_value will be set accordingly
 */
bool syscall_wait(u_int32 *pid, u_int32 *return_value);


void syscall_printf(string s, ...);

void syscall_hlt();


#endif
