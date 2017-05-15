#ifndef LIB_H
#define LIB_H


#define NULL  0

typedef unsigned int   u_int32;
typedef   signed int   s_int32;
typedef unsigned short u_int16;
typedef   signed short s_int16;
typedef unsigned char  u_int8;
typedef   signed char  s_int8;

typedef u_int32* fd;

typedef unsigned char bool;
#define FALSE 0
#define TRUE  1


typedef char* string;


/**
 * @name syscall_fork - Creates a new process with a new, copied context
 * @param priority    - The priority to give to the child process
 * @param pid         - A pointer toward an integer that will be set with a process id
 * @return u_int32    - 0 if the fork failed (pid has not been modified)
 *                      1 for the parent process, pid contains the id of the child process
 *                      2 for the child process, pid contains the id of the parent process
 */
u_int32 syscall_fork(u_int32 priority, u_int32 *pid);

/**
 * @name syscall_exit  - Terminates the process
 * The child of the process have their parent replaced by the init process, while
 * the parent of the process will be given (once he calls wait) the return value.
 * @param return_value - The return value of the exiting process
 * @return void
 */
void syscall_exit(u_int32 return_value);

/**
 * @name syscall_wait  - Waits for a child process to call exit
 * @param pid          - Will contain the pid of the exited child process
 * @param return_value - Will contain the return value of the exited child process
 * @return bool        - 0 if the process has no children (pid and return_value won't be modified)
 *                       1 if the call succeeded, and pid and return_value will be set accordingly
 */
bool syscall_wait(u_int32 *pid, u_int32 *return_value);


void syscall_printf(string s, ...);

void syscall_hlt();

void *syscall_malloc(u_int32 size);
void syscall_free(void *ptr);

#endif
