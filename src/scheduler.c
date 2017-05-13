#include "scheduler.h"
#include "malloc.h"
#include "queue.h"
#include "timer.h"
#include "irq.h"
#include "syscall.h"
#include "paging.h"
#include "filesystem.h"
#include "elf.h"


scheduler_state_t *state = NULL;


void select_new_process()
{
  /* Search for a runnable process */
  bool found = FALSE;
  queue_t *temp = empty_queue();  /* The queue to temporary save processes into */

  for (priority prio = MAX_PRIORITY; prio >= 0 && !found; prio--) {
    queue_t *q = state->runqueues[prio];

    while (!is_empty_queue(q) && !found) {
      pid pid = dequeue(q);
      enqueue(temp, pid);

      if (state->processes[pid].state == Runnable) {
        /* We found a runnable process */
        found = TRUE;
        state->curr_pid = pid;
      }
    }

    /* Restoring the runqueue */
    while (!is_empty_queue(temp)) {
      enqueue(q, dequeue(temp));
    }
    /* The selected process is at the end of his runqueue now */
  }

  mem_free(temp);
}


/* Context-switching defines, not functions, to avoid using the stack */
#define SWITCH_BEFORE() {                                               \
    /* Restores kernel paging */                                        \
    switch_page_directory(kernel_directory);                            \
    context_t *ctx = &state->processes[state->curr_pid].context;        \
                                                                        \
    /* Saves process context */                                         \
    asm volatile ("mov %%esp, %0" : "=r" (ctx->esp));                   \
    ctx->regs = regs;                                                   \
    ctx->first_free_block = first_free_block;                           \
    ctx->unallocated_mem  = unallocated_mem;                            \
                                                                        \
    /* Restores kernel context */                                       \
    unallocated_mem  = kernel_context.unallocated_mem;                  \
    first_free_block = kernel_context.unallocated_mem;                  \
  }

#define SWITCH_AFTER() {                                                \
    /* Saves kernel context */                                          \
    kernel_context.unallocated_mem  = unallocated_mem;                  \
    kernel_context.first_free_block = first_free_block;                 \
                                                                        \
    /* Restores process context */                                      \
    context_t *ctx = &state->processes[state->curr_pid].context;        \
    first_free_block = ctx->first_free_block;                           \
    unallocated_mem  = ctx->unallocated_mem;                            \
    /* No need to touch at the regs structure because of pointers */    \
    asm volatile ("mov %0, %%esp" : : "r" (ctx->esp));                  \
                                                                        \
    /* Restores process paging */                                       \
    switch_page_directory(ctx->page_dir);                               \
  }

void timer_handler(regs_t *regs)
{
  SWITCH_BEFORE();       /* Save context + kernel paging */
  select_new_process();
  SWITCH_AFTER();        /* Paging set-up + restore the stack and the context */
}

/**
 * @name syscall_handler - Handler for the syscall interruption
 * Decodes the syscall (in eax) and performs it on the current state.
 * @param regs           - Context of the current process
 * @return void
 */
void syscall_handler(regs_t *regs)
{
  SWITCH_BEFORE();  /* Save context + kernel paging */

  switch (regs->eax) { /* Syscall number */

  case Exit:
    syscall_exit();
    break;

  case Fork:
    syscall_fork();
    break;

  case Wait:
    syscall_wait();
    break;

  default:
    syscall_invalid();
  }

  /* Check if the syscall has not ended, and if it is the case select a new process */
  if (state->processes[state->curr_pid].state != Runnable) {
    select_new_process();
  }

  SWITCH_AFTER();  /* Paging set-up + restore the stack and the context */
}


void load_code(string program_name, context_t ctx)
{
  kloug(100, "Loading %s code\n", program_name);

  string temp = str_cat("/progs/", program_name);
  string path = str_cat(temp, ".elf");
  mem_free(temp);

  u_int32 inode = find_inode(path, 0);
  mem_free(path);

  /* TODO: read until EOF or something */
  inode_t inode_buffer;
  set_inode(inode, &inode_buffer);
  size_t size = inode_buffer.sectors * 512;  /* Each sector amounts to 512 bytes */

  /* Loads ELF file in memory */
  u_int8 *elf_buffer = (u_int8*) mem_alloc(size);
  u_int8 *current = elf_buffer;
  while (current < elf_buffer + size) {
    current += read_inode_data(inode, current, 0, elf_buffer + size - current);
  }

  u_int32 nb_pages = (0xFFFFFFFF - START_OF_USER_CODE + 1) / 0x1000;
  /* kloug(100, "%d pages of user code\n", nb_pages); */
  u_int32 virtuals[nb_pages];
  for (u_int32 page = 0; page < nb_pages; page++) {
    u_int32 physical = get_physical_address(ctx.page_dir, START_OF_USER_CODE + page*0x1000);
    u_int32 virtual = request_physical_space(current_directory, physical, TRUE, FALSE);
    if (virtual) {
      virtuals[page] = virtual;
    } else {
      /* TODO: free properly */
      throw("Unable to load user code");
    }
  }

  /* Loads actual code and data at the right place, and set up eip */
  ctx.regs->eip = check_and_load(elf_buffer, virtuals);

  /* Free! */
  mem_free(elf_buffer);
  for (u_int32 page = 0; page < nb_pages; page++) {
    free_virtual_space(current_directory, virtuals[page], FALSE);  /* Frame also mapped for the proc */
  }
}


/**
 * @name switch_to_process - Transfers control to the given new process
 * This ignores the regs structure, as well as the kernel esp
 * @param pid              - The process to transfer control to
 * @return void
 */
void switch_to_process(pid pid)
{
  process_t proc = state->processes[pid];

  /* Saves kernel context */
  kernel_context.unallocated_mem  = unallocated_mem;
  kernel_context.first_free_block = first_free_block;
  asm volatile ("mov %%esp, %0" : "=r" (kernel_context.esp));

  /* Restores process context */
  context_t ctx = proc.context;
  first_free_block = ctx.first_free_block;
  unallocated_mem  = ctx.unallocated_mem;

  /* Pushes the regs structure on the stack */
  kloug(100, "Pushing, kernel ESP %X user ESP %X EIP %X\n",       \
        kernel_context.esp, 8, ctx.regs->useresp, 8, ctx.regs->eip, 8);

  /* Simulate an interruption stack frame */
  asm volatile ("push %0" : : "r" (ctx.regs->ss));
  asm volatile ("push %0" : : "r" (ctx.regs->useresp));
  asm volatile ("push %0" : : "r" (ctx.regs->eflags));
  asm volatile ("push %0" : : "r" (ctx.regs->cs));
  asm volatile ("push %0" : : "r" (ctx.regs->eip));

  /* The user segments */
  asm volatile ("push %0" : : "r" (ctx.regs->ds));
  asm volatile ("push %0" : : "r" (ctx.regs->es));
  asm volatile ("push %0" : : "r" (ctx.regs->fs));
  asm volatile ("push %0" : : "r" (ctx.regs->gs));

  /* Restores process paging */
  kloug(100, "Oh god\n");
  log_page_dir(ctx.page_dir);
  switch_page_directory(ctx.page_dir);
  kloug(100, "Are we still here?\n");

  /* Let's go! */
  asm volatile ("pop %gs");
  asm volatile ("pop %fs");
  asm volatile ("pop %es");
  asm volatile ("pop %ds");
  asm volatile ("iret");        /* Pops the 5 last things */
}


void run_program(string name)
{
  pid pid = 0;
  while (pid < NUM_PROCESSES && state->processes[pid].state != Free) {
    pid++;
  }

  if (pid == NUM_PROCESSES) {
    writef("%frun;%f\tUnable to create a new process\n", LightRed, White);
    return;
  }

  process_t *proc = &state->processes[pid];
  *proc = new_process(pid, 1);  /* User processes have a priority of 1 */
  load_code(name, proc->context);

  switch_to_process(pid);
}


void scheduler_install()
{
  state = mem_alloc(sizeof(scheduler_state_t));

  /* Creating idle process */
  pid idle_pid = 0;
  process_t *idle = &state->processes[idle_pid];
  *idle = new_process(idle_pid, 0);
  load_code("idle", idle->context);

  /* Creating init process */
  pid init_pid = 1;
  process_t *init = &(state->processes[init_pid]);
  *init = new_process(init_pid, MAX_PRIORITY);
  load_code("init", init->context);

  /* Initialization of the state */
  state->curr_pid = init_pid;  /* We start with the init process */
  for (priority prio = 0; prio <= MAX_PRIORITY; prio++) {
    state->runqueues[prio] = empty_queue();
  }
  /* Adds idle and init in the runqueues */
  enqueue(state->runqueues[0],            idle_pid);
  enqueue(state->runqueues[MAX_PRIORITY], init_pid);

  /* Adds handlers for timer and syscall interruptions */
  extern void *timer_phase(int hz);  /* Defined in timer.c */
  timer_phase(SWITCH_FREQ);
  /* irq_install_handler(0, timer_handler); */
  /* isr_install_handler(SYSCALL_ISR, syscall_handler); */

  kloug(100, "Scheduler installed\n");
}
