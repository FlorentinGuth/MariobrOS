#ifndef SYSCALL_H
#define SYSCALL_H

#include "scheduler.h"
#include "fs_inter.h"

#define SYSCALL_ISR 0x80

typedef enum syscall {
  Exit       =  0,    /* The process is finished and returns a value */
  Fork       =  1,    /* Creates a new child process, with the same context at first */
  Wait       =  2,    /* Waits for a child to return a value */
  Printf     =  3,    /* Prints to the framebuffer */
  Malloc     =  4,
  MemFree    =  5,
  Ls         =  6,
  Rm         =  7,
  Mkdir      =  8,
  Keyget     =  9,
  Run        = 10,
  Hlt        = 11,
  NewChannel = 12,
  Send       = 13,
  Receive    = 14,
  Open       = 15,
  Close      = 16,
  Read       = 17,
  Write      = 18,
  Lseek      = 19,
  Fstat      = 20,
  Set_curs   = 21,
  Get_curs   = 22,
  Scroll     = 23,
  Write_box  = 24,
  Gcwd       = 25,
  Find_dir   = 26,
  Clear_buf  = 27,
  Invalid,       /* /!\ This need to be the last syscall */
} syscall_t;

/**
 * @name syscall_fork - Creates a new process with a new, copied context
 * This syscall has one param, in ebx: the priority to give to the child process,
 * which must be less than or equal to the priority of the current process.
 * If there's no free process, or if the child priority is higher than the priority
 * of the current process, the call terminates and places 0 in eax.
 * Otherwise, the parent process has 1 in eax and the pid of the child process in ebx,
 * while the child process has 2 in eax and the pid of the parent process in ebx.
 * @return void
 */
void syscall_fork();
/**
 * @name syscall_exit  - Terminates the process
 * This syscall has one param, in ebx: the return value of the exiting process.
 * The child of the process have their parent replaced by the init process, while
 * the parent of the process will be given (once he calls wait) the return value.
 * @return void
 */
void syscall_exit();
/**
 * @name syscall_wait - Waits for a child process to call exit
 * If the process has no child processes, 0 will be placed in eax.
 * Otherwise, 1 is placed in eax, the pid of the exiting child process in ebx
 * and its return value in ecx.
 * @return void
 */
void syscall_wait();

void resolve_exit_wait(pid parent, pid child);

/**
 * @name syscall_printf - Prints to the framebuffer (stdout)
 * The address of the format string is located in ebx, while the different
 * arguments are pushed on top of the stack (in reverse order, i.e. the one on
 * top is the leftmost).
 * @return void
 */
void syscall_printf();

void syscall_invalid();


/**
 * @name syscall_install - Installs the syscall table
 * @return void
 */
void syscall_install();


/**
 * @name syscall - Decodes and performs syscall
 * @param sc     -
 * @return void
 */
void syscall(syscall_t sc);

#endif
