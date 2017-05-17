#include "syscall.h"
#include "error.h"
#include "scheduler.h"
#include "queue.h"
#include "logging.h"
#include "syscall_asm.h"
#include "idt.h"
#include "gdt.h"
#include "malloc.h"
#include "memory.h"
#include "shell.h"
#include "utils.h"


/* Possible speed enhancements:
 * - List of child processes for each process
 * - Stack of free processes
 */

u_int8 sys_buf[2048]; // Static buffer

extern scheduler_state_t *state;  /* Defined in scheduler.c */
extern pid run_pid;

#define CURR_PROC (state->processes[state->curr_pid])
#define CURR_REGS (state->processes[state->curr_pid].context.regs)

#define SWITCH_AFTER()                                              \
  kernel_context.unallocated_mem  = unallocated_mem;                \
  kernel_context.first_free_block = first_free_block;               \
  context_t *ctx = &state->processes[state->curr_pid].context;      \
  first_free_block = ctx->first_free_block;                         \
  unallocated_mem  = ctx->unallocated_mem;                          \
  switch_page_directory(ctx->page_dir);



#define SWITCH_BEFORE()                                 \
  switch_page_directory(kernel_directory);              \
  ctx->first_free_block = first_free_block;             \
  ctx->unallocated_mem  = unallocated_mem;              \
  unallocated_mem  = kernel_context.unallocated_mem;    \
  first_free_block = kernel_context.first_free_block;

void syscall_malloc()
{
  /* log_page_dir(CURR_PROC.context.page_dir); */
  size_t size = CURR_REGS->ebx;
  SWITCH_AFTER();
  /* log_memory(); */
  /* u_int32 esp; asm volatile ("mov %%esp, %0" : "=r" (esp)); */
  /* kloug(100, "Allocating %x bytes, ESP=%X\n", size, esp, 8); */
  u_int32 ret = (u_int32)mem_alloc(size);
  /* kloug(100, "Returned %X\n", ret, 8); */
  SWITCH_BEFORE();
  CURR_REGS->eax = ret;
}

void syscall_free()
{
  void *ptr = (void *)CURR_REGS->ebx;
  SWITCH_AFTER();
  mem_free(ptr);
  SWITCH_BEFORE();
}

void syscall_open()
{
  string path   = (void*) CURR_REGS->ebx;
  u_int8 oflag  = CURR_REGS->ecx & 0xFF;
  u_int16 fperm = CURR_REGS->edx & 0xFFFF;
  SWITCH_AFTER();
  str_copy(path, (void*) &sys_buf);
  SWITCH_BEFORE();
  fd ret = openfile((void*) &sys_buf, oflag, fperm);
  CURR_REGS->eax = (u_int32) ret;
}

void syscall_close()
{
  fd f = (void*) CURR_REGS->ebx;
  close(f);
}

void syscall_read()
{
  fd f = (void*) CURR_REGS->ebx;
  u_int8* buffer = (void*) CURR_REGS->ecx;
  u_int32 offset = CURR_REGS->edx;
  u_int32 length = CURR_REGS->edi;
  if(!f || *f > fdt_size || fdt[*f].this != f || !fdt[*f].inode) {
    CURR_REGS->eax = 0; // Invalid file descriptor
    return;
  }
  if(!(fdt[*f].mode & O_RDONLY)) {
    CURR_REGS->eax = 0; // No permission
    return;
  }
  if(fdt[*f].pos >= fdt[*f].size) {
    CURR_REGS->eax = 0;
    return;
  }
  if(fdt[*f].pos + length > fdt[*f].size) {
    length = fdt[*f].size - fdt[*f].pos;
  }
  u_int32 inode = fdt[*f].inode;
  u_int32 pos = fdt[*f].pos;
  u_int32 done = read_inode_data(inode, sys_buf, pos, length);
  SWITCH_AFTER();
  mem_copy(buffer + offset, sys_buf, done);
  SWITCH_BEFORE();
  fdt[*f].pos += done;
  CURR_REGS->eax = done;
}

void syscall_write()
{
  fd f = (void*) CURR_REGS->ebx;
  u_int8* buffer = (void*) CURR_REGS->ecx;
  u_int32 offset = CURR_REGS->edx;
  u_int32 length = CURR_REGS->edi;
  if(!f || *f > fdt_size || fdt[*f].this != f || !fdt[*f].inode) {
    CURR_REGS->eax = 0;
    return; // Invalid file descriptor
  }
  if(!(fdt[*f].mode & O_WRONLY)) {
    CURR_REGS->eax = 0;
    return; // No permission
  }
  if(!length) {
    CURR_REGS->eax = 0;
    return;
  }
  u_int32 to_use = 1 + (fdt[*f].pos + length - 1) / block_size;
  u_int32 used;
  if(fdt[*f].size) {
    used = 1 + (fdt[*f].size - 1) / block_size;
  } else {
    used = 0;
  }

  u_int32 alloc  = prepare_blocks(fdt[*f].inode, used, to_use);
  if(to_use > used && alloc != to_use - used) {
    CURR_REGS->eax = 0;
    return; // All the blocks could not be allocated
  }
  u_int32 written = 0;
  u_int32 done = 0;
  u_int32 inode = fdt[*f].inode;
  u_int32 pos = fdt[*f].pos;
  while(written != length) {
    SWITCH_AFTER();
    mem_copy(sys_buf, buffer + offset, max(length - written, sizeof(sys_buf)));
    SWITCH_BEFORE();
    done = write_inode_data(inode, sys_buf, pos, length - written);
    if(!done) { // No data was written this time, so it won't evolve
      break;
    }
    offset += done;
    pos += done;
    written += done;
  }
  fdt[*f].pos = pos;
  if(pos > fdt[*f].size) {
    fdt[*f].size = pos;
    // std_inode was set by write_inode_data or by prepare_block
    std_inode->size_low = fdt[*f].size;
    update_inode(fdt[*f].inode, std_inode);
  }

  CURR_REGS->eax = written;
}

void syscall_lseek()
{
  fd      f      = (void*) CURR_REGS->ebx;
  s_int32 offset = (s_int32) CURR_REGS->ecx;
  u_int8  seek   = (u_int8)  CURR_REGS->edx;
  CURR_REGS->eax = lseek(f, offset, seek);
}

void syscall_fstat()
{
  fd f = (void*) CURR_REGS->ebx;
  fstat(f, (void*) &sys_buf);
  SWITCH_AFTER();
  stats* s = (void*) CURR_REGS->ecx;
  *s = *((stats*) (&sys_buf));
  SWITCH_BEFORE();
}


void syscall_fork()
{
  kloug(100, "Syscall fork\n");

  process_t *parent = &state->processes[state->curr_pid];
  priority child_prio = CURR_REGS->ebx;

  /* Research of a free process */
  pid id = 0;
  while (id < NUM_PROCESSES && state->processes[id].state != Free) {
    id += 1;
  }

  /* One cannot create a child process with a higher priority than its own */
  if (id == NUM_PROCESSES || child_prio > CURR_PROC.prio) {
    CURR_REGS->eax = 0;
    return;
  }

  /* Initialization of fields, registers, copying of context */
  process_t *proc = &state->processes[id];
  *proc = new_process(state->curr_pid, child_prio, FALSE);
  /* Context */
  mem_copy(&proc->context, &parent->context, sizeof(context_t));
  /* Regs */
  proc->context.regs = (regs_t *)mem_alloc(sizeof(regs_t));
  /* kloug(100, "Parent %x child %x\n", parent->context.regs, proc->context.regs); */
  mem_copy(proc->context.regs, parent->context.regs, sizeof(regs_t));
  proc->context.regs->eax = 2;
  proc->context.regs->ebx = state->curr_pid;
  /* Page directory */
  proc->context.page_dir = fork_page_dir(parent->context.page_dir);

  /* Adding the process in the runqueue */
  enqueue(state->runqueues[child_prio], id);

  /* Setting the values of the parent process */
  CURR_REGS->eax = 1;
  CURR_REGS->ebx = id;
}

/**
 * @name resolve_exit_wait - Resolves an exit or wait syscall
 * @param parent           - The parent process, in waiting mode
 * @param child            - The child process, in zombie mode
 * @return void
 */
void resolve_exit_wait(pid parent, pid child)
{
  kloug(100, "Resolve exit wait %d %d\n", parent, child);
  process_t* parent_proc = &state->processes[parent];
  process_t* child_proc  = &state->processes[child];
  /* kloug(100, "Child ebx %d\n", child_proc->context.regs->ebx); */
  /* kloug(100, "%x %x\n", parent_proc->context.regs, child_proc->context.regs); */
  /* Freeing the child from zombie state */
  state->processes[child].state = Free;

  /* Goodbye cruel world: removes the child from the runqueue */
  priority prio = child_proc->prio;
  queue_t *q = state->runqueues[prio];
  queue_t *temp = empty_queue();
  while (!is_empty_queue(q)) {
    pid pid = dequeue(q);
    if (pid != child) {
      enqueue(temp, pid);
    }
  }
  state->runqueues[prio] = temp;

  /* Also free everything */
  mem_free(child_proc->context.regs);
  free_page_dir(child_proc->context.page_dir);

  /* Notifies the parent */
  parent_proc->state = Runnable;
  parent_proc->context.regs->eax = 1;
  parent_proc->context.regs->ebx = child;
  parent_proc->context.regs->ecx = child_proc->context.regs->ebx;  /* Return value */

  /* Take care of run command */
  if (run_pid == child) {
    run_pid = 0;
    finalize_command();
  }
}

void syscall_exit()
{
  kloug(100, "Syscall exit\n");

  pid id = state->curr_pid;
  state->processes[id].state = Zombie;

  /* Notifies the child processes of the exiting one, and resolve also exits */
  for (pid i = 0; i < NUM_PROCESSES; i++) {
    if (state->processes[i].parent_id == id) {
      state->processes[i].parent_id = 1;
      if (state->processes[i].state == Zombie && state->processes[1].state == Waiting) {
        resolve_exit_wait(1, i);
      }
    }
  }

  /* Checks whether the parent was waiting for us to die (how cruel!) */
  pid parent_id = state->processes[id].parent_id;
  if (state->processes[parent_id].state == Waiting) {
    resolve_exit_wait(parent_id, id);
  }
}

void syscall_wait()
{
  kloug(100, "Syscall wait\n");

  CURR_PROC.state = Waiting;
  pid parent_id = state->curr_pid;
  bool has_children = FALSE;

  for (pid id = 0; id < NUM_PROCESSES; id++) {
    process_t proc = state->processes[id];
    if (proc.parent_id == parent_id) {
      has_children = TRUE;
      if (proc.state== Zombie) {
        resolve_exit_wait(parent_id, id);
        return;
      }
    }
  }

  if (!has_children) {
    /* The process has no children, the call terminates instantly */
    CURR_REGS->eax = 0;
    CURR_PROC.state = Runnable;
  }
}


extern unsigned char utf8_c2[], utf8_c3[];  /* Defined in printer.c */
extern color_t foreground, background;      /* Defined in printer.c */

#define POP(type)                                                       \
  type param = *(type *)(esp + 16 + 4*nb_args);                         \
  nb_args++;


void syscall_printf()
{
  context_t ctx = CURR_PROC.context;
  u_int32 esp = ctx.regs->useresp;
  string s = (string)ctx.regs->ebx;
  switch_page_directory(ctx.page_dir);

  /* kloug(100, "Format string %s\n", s); */

  int read  = 0;
  char buffer[17];
  char c = s[0];
  int nb_args = 0; /* To have the right offset */

  while(c!='\0') {
    if(c=='%') {
      read++;
      switch(s[read]) {

      case 'd': { // Decimal (signed)
        POP(int);
        int_to_string(buffer, param, 10);
        write_string(buffer); break; }
      case 'u': { // Decimal (unsigned)
        POP(unsigned int);
        u_int_to_string(buffer, param, 10);
        write_string(buffer); break; }
      case 'x': { // Hexadecimal
        POP(unsigned int);
        u_int_to_string(buffer, param, 16);
        write_string("0x"); write_string(buffer); break; }
      case 'h': { // Hexadecimal (without "0x")
        POP(unsigned int);
        u_int_to_string(buffer, param, 16);
        write_string(buffer); break; }
      case 'c': { // Character
        POP(char);
        write_char(param); break; }
      case 's': { // String
        POP(string);
        write_string(param); break; }
      case 'f': { // Foreground color
        POP(color_t);
        foreground = param; break; }
      case 'b': { // Background color
        POP(color_t);
        background = param; break; }
      case '%': { // Writes a '%'
        write_char('%'); break; }
      default:  { // Emergency stop
        throw("Invalid format string");
      }
      }
    } else if(c==0xc2) {
      read++;
      write_char(utf8_c2[(unsigned int)(s[read]-0xa1)]);
    }
    else if(c==0xc3) {
      read++;
      write_char(utf8_c3[(unsigned int)(s[read]-0x80)]);
    }
    else
      write_char(c);
    read++;
    c = s[read];
  }

  switch_page_directory(kernel_directory);
}


extern bool in_kernel, should_cycle, run_executed;  /* From scheduler.c */
void syscall_hlt()
{
  should_cycle = FALSE;

  for (;;) {
    asm volatile ("sti; hlt; cli");

    if (!should_cycle || run_executed) {
      /* The interruption was not a timer */
      should_cycle = TRUE;  /* We want to change process, maybe someone has something to do */
      run_executed = FALSE;
      /* kloug(100, "Leaving hlt\n"); */
      break;
    } else {
      /* We received a timer event */
      /* kloug(100, "Timer event\n"); */
      should_cycle = FALSE;
    }
  }
}


void syscall_invalid()
{
  throw("Invalid syscall!");
}


#define NUM_SYSCALLS Invalid
/* Brace yourselves: an array of function pointers */
void (*syscall_table[NUM_SYSCALLS])();

void syscall_install()
{
  syscall_table[Exit]    = *syscall_exit;
  syscall_table[Wait]    = *syscall_wait;
  syscall_table[Fork]    = *syscall_fork;
  syscall_table[Printf]  = *syscall_printf;
  syscall_table[Hlt]     = *syscall_hlt;
  syscall_table[Malloc]  = *syscall_malloc;
  syscall_table[MemFree] = *syscall_free;
  syscall_table[Open]    = *syscall_open;
  syscall_table[Close]   = *syscall_close;
  syscall_table[Read]    = *syscall_read;
  syscall_table[Write]   = *syscall_write;
  syscall_table[Lseek]   = *syscall_lseek;
  syscall_table[Fstat]   = *syscall_fstat;

  idt_set_gate(SYSCALL_ISR, (u_int32)common_interrupt_handler, KERNEL_CODE_SEGMENT, 3);
}

void syscall(syscall_t sc)
{
  if (sc >= NUM_SYSCALLS) {
    syscall_invalid();
  } else {
    syscall_table[sc]();
  }
}
