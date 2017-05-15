#include "syscall.h"
#include "error.h"
#include "scheduler.h"
#include "queue.h"
#include "logging.h"
#include "syscall_asm.h"
#include "idt.h"
#include "gdt.h"


/* Possible speed enhancements:
 * - List of child processes for each process
 * - Stack of free processes
 */

extern scheduler_state_t *state;  /* Defined in scheduler.c */

#define CURR_PROC (state->processes[state->curr_pid])
#define CURR_REGS (state->processes[state->curr_pid].context.regs)

void syscall_malloc()
{
  SWITCH_AFTER();
  CURR_REGS->eax = mem_alloc(CURR_REGS->ebx);
  SWITCH_BEFORE();
}

void syscall_free()
{
  SWITCH_AFTER();
  mem_free(CURR_REGS->ebx);
  SWITCH_BEFORE();
}

void syscall_ls()
{
  
}

void syscall_fork()
{
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
  proc->parent_id = state->curr_pid;
  proc->state = Runnable;
  proc->context.regs->eax = 2;
  proc->context.regs->ebx = state->curr_pid;
  /* TODO: copy context */

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
  /* Freeing the child from zombie state */
  state->processes[child].state= Free;

  /* Goodbye cruel world: removes the child from the runqueue */
  priority prio = state->processes[child].prio;
  queue_t *q = state->runqueues[prio];
  queue_t *temp = empty_queue();
  while (!is_empty_queue(q)) {
    pid pid = dequeue(q);
    if (pid != child) {
      enqueue(temp, pid);
    }
  }
  state->runqueues[prio] = temp;

  /* Notifies the parent */
  process_t* parent_proc = &state->processes[parent];
  parent_proc->state= Runnable;
  parent_proc->context.regs->eax = 1;
  parent_proc->context.regs->ebx = child;
  parent_proc->context.regs->ecx = state->processes[child].context.regs->ebx;  /* Return value */
}

void syscall_exit()
{
  kloug(100, "Syscall exit\n");

  pid id = state->curr_pid;
  state->processes[id].state = Zombie;

  /* Notifies the child processes of the exiting one */
  for (pid i = 0; i < NUM_PROCESSES; i++) {
    if (state->processes[i].parent_id == id) {
      state->processes[i].parent_id = 1;
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
  kloug(100, "User esp %X\n", esp, 8);
  switch_page_directory(ctx.page_dir);

  string s = (string)ctx.regs->ebx;
  kloug(100, "Format string %s\n", s);

  int read  = 0;
  char buffer[17];
  char c = s[0];
  int nb_args = 0; /* To have the right offset */

#define PRINT(n) kloug(10, "%u ", *(u_int32 *)(esp + n))
  PRINT(0); PRINT(4); PRINT(8); PRINT(12); PRINT(16); PRINT(20);

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


void syscall_invalid()
{
  throw("Invalid syscall!");
}


#define NUM_SYSCALLS Invalid
/* Brace yourselves: an array of function pointers */
void (*syscall_table[NUM_SYSCALLS])();

void syscall_install()
{
  syscall_table[Exit]   = *syscall_exit;
  syscall_table[Wait]   = *syscall_wait;
  syscall_table[Fork]   = *syscall_fork;
  syscall_table[Printf] = *syscall_printf;
  syscall_table[Malloc] = *syscall_malloc;
  syscall_table[Free]   = *syscall_free;

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
