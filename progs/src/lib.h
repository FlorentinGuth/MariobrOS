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

typedef struct stats {
  u_int32 st_ino;
  u_int8  st_kind;
  u_int16 st_perm;
  u_int16 st_nlink;
  u_int32 st_size;
} stats;

/**
 *  @name fd - The type of the file descriptors
 */
typedef u_int32* fd;

typedef unsigned char bool;
#define FALSE 0
#define TRUE  1

/**
 *  @name pos_t - The type of a position on the screen (whose size is 80*25)
 *  Note that a pos_t never accounts for the fact that each location takes up two bytes.
 */
typedef u_int16 pos_t;

enum Color {
  Black        =  0,
  Blue         =  1,
  Green        =  2,
  Cyan         =  3,
  Red          =  4,
  Magenta      =  5,
  Brown        =  6,
  LightGrey    =  7,
  DarkGrey     =  8,
  LightBlue    =  9,
  LightGreen   = 10,
  LightCyan    = 11,
  LightRed     = 12,
  LightMagenta = 13,
  LightBrown   = 14,
  White        = 15,
};
typedef enum Color color_t;

#define SCREEN_WIDTH  80
#define SCREEN_HEIGHT 25
#define TAB_WIDTH      4

#define POS(row, col)  ((row) * SCREEN_WIDTH + (col))

typedef char* string;


void *malloc(u_int32 size);

void free(void *ptr);

/**
 *  @name open - Opens a file
 *
 *  @param path        - The path to the file
 *  @param oflag       - The open flags
 *  @param perm        - The file access rights
 *
 *  @return            - The file descriptor for the given file or 0 if an error
 *  occured
 */
fd open(string path, u_int32 oflag, u_int32 fperm);

/**
 *  @name close - Closes a file descriptor, ensuring it is no more valid
 *  @param f            - The file descriptor to close
 */
void close(fd f);

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
u_int32 write(fd f, u_int8* buffer, u_int32 offset, u_int32 length);

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
 *  @name fstat - Sets stats of a file
 *  @param f    - The file descriptor
 *  @param s    - A pointer to a stats structure
 */
void fstat(fd f, stats* s);

/**
 *  @name fork - Creates a new process with a new, copied context
 *  @param priority    - The priority to give to the child process
 *  @param pid         - A pointer toward an integer that will be set with a process id
 *  @return u_int32    - 0 if the fork failed (pid has not been modified)
 *                       1 for the parent process, pid contains the id of the child process
 *                       2 for the child process, pid contains the id of the parent process
 */
u_int32 fork(u_int32 priority, u_int32 *pid);

/**
 *  @name exit  - Terminates the process
 *  The child of the process have their parent replaced by the init process, while
 *  the parent of the process will be given (once he calls wait) the return value.
 *  @param return_value - The return value of the exiting process
 *  @return void
 */
void exit(u_int32 return_value);

/**
 *  @name scwait  - Waits for a child process to call exit
 *  @param pid          - Will contain the pid of the exited child process
 *  @param return_value - Will contain the return value of the exited child process
 *  @return bool        - 0 if the process has no children (pid and return_value won't be modified)
 *                        1 if the call succeeded, and pid and return_value will be set accordingly
 */
bool scwait(u_int32 *pid, u_int32 *return_value);


/**
 *  @name set_cursor_pos - Moves the cursor of the framebuffer.
 *
 *  @param pos The new position of the cursor
 */
void set_cursor_pos(pos_t pos);

/**
 *  @name get_cursor_pos -
 *  Communicate with the framebuffer to get the cursor position.
 *
 *  @return The position of the cursor
 */
pos_t get_cursor_pos();

/**
 *  @name ls        - List directory contents
 *  @param dir      - The name of the directory
 *  @param curr_dir - If dir does not begin with '/', local root
 */
void ls(string dir, u_int32 curr_dir);

/**
 *  @name rm        - Removes a file or a directory
 *
 *  @param name     - The file to remove
 *  @param curr_dir - Local root
 *  @param rec      - If set, recursively deletes a directory and its contents
 *
 *  @return error   - 0: No error occured
 *                    1: File does not exist
 *                    2: Cannot delete the directory
 *                    3: rec is not set but name designate a directory
 *                    4: Unknown error
 */
u_int8 rm(string name, u_int32 curr_dir, bool rec);

/**
 *  @name mkdir     - Creates a directory
 *  @param dir      - The name of the directory to create
 *  @param curr_dir - Local root
 *  @retur error    - 0: No error occured, 1: Impossible to create directory
 */
bool mkdir(string dir, u_int32 curr_dir);

/**
 *  @name get_cwd   - Gets the path to the current working directory
 *  @param curr_dir - Current working directory
 *  @param path     - The current path (to modify)
 *  @return name    - The new path, or 0 if an error occured
 */
string get_cwd(u_int32 curr_dir, string path);

/**
 *  @name find_dir  - Gives the inode number of the selected directory
 *  @param path     - The local or global path to the directory
 *  @param curr_dir - If path does not begin with '/', local root
 *  @return         - The inode number
 */
u_int32 find_dir(string path, u_int32 curr_dir);

/**
 *  @name scroll - Move all the text one line up, and erases the last line.
 */
void scroll();

/**
 *  @name set_char - Writes a physical character in the framebuffer.
 *
 *  @param i  - The location in the framebuffer
 *  @param c  - The ASCII character 
 */
void set_char(pos_t i, char c);

/**
 *  @name get_char - Gets the physical character from the framebuffer
 *  @param i       - The location in the framebuffer
 *  @return        - The character
 */
char get_char(pos_t i);

/**
 *  @name keyboard_get - Gives the first non read scancode (if shell is on)
 */
u_int8 keyget();


void printf(string s, ...);

void hlt();

void run_program(string path);

#endif
