#include "scheduler.h"
#include "malloc.h"
#include "memory.h"
#include "queue.h"
#include "timer.h"
#include "irq.h"
#include "syscall.h"
#include "paging.h"
#include "filesystem.h"
#include "elf.h"
#include "list.h"


scheduler_state_t *state = NULL;
bool in_kernel = FALSE;     /* If true, we were doing a syscall while we were interrupted */
bool should_cycle = FALSE;  /* If true, we should select a new process after the current syscall */


void select_new_process()
{
  /* kloug(100, "Select new process\n"); */

  /* Search for a runnable process */
  bool found = FALSE;
  queue_t *temp = empty_queue();  /* The queue to temporary save processes into */

  for (priority prio = MAX_PRIORITY; prio >= 0 && !found; prio--) {
    queue_t *q = state->runqueues[prio];

    while (!is_empty_queue(q) && !found) {
      pid pid = dequeue(q);
      enqueue(temp, pid);

      /* kloug(100, "Process %d is %d\n", pid, state->processes[pid].state); */

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

  /* kloug(100, "Selected %d as new process\n", state->curr_pid); */
}


/* Context-switching defines, not functions, to avoid using the stack */
#define SWITCH_BEFORE() {                                               \
    in_kernel = TRUE;                                                   \
    /* Restores kernel paging */                                        \
    switch_page_directory(kernel_directory);                            \
    context_t *ctx = &state->processes[state->curr_pid].context;        \
                                                                        \
    /* Saves process context */                                         \
    *ctx->regs = *regs;                                                 \
    ctx->first_free_block = first_free_block;                           \
    ctx->unallocated_mem  = unallocated_mem;                            \
                                                                        \
    /* Restores kernel context */                                       \
    unallocated_mem  = kernel_context.unallocated_mem;                  \
    first_free_block = kernel_context.first_free_block;                 \
  }

#define SWITCH_AFTER() {                                                \
    /* kloug(100, "Switching back to %d\n", state->curr_pid);  */       \
    /* Saves kernel context */                                          \
    kernel_context.unallocated_mem  = unallocated_mem;                  \
    kernel_context.first_free_block = first_free_block;                 \
                                                                        \
    /* Restores process context */                                      \
    context_t *ctx = &state->processes[state->curr_pid].context;        \
    first_free_block = ctx->first_free_block;                           \
    unallocated_mem  = ctx->unallocated_mem;                            \
    *regs = *ctx->regs;                                                 \
                                                                        \
    /* Restores process paging */                                       \
    switch_page_directory(ctx->page_dir);                               \
    in_kernel = FALSE;                                                  \
  }

void timer_handler(regs_t *regs)
{
  /* kloug(100, "Timer\n"); */

  if (in_kernel) {
    /* kloug(100, "Timer during kernel!\n"); */
    should_cycle = TRUE;
    return;
  }

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
  if (in_kernel) {
    throw("WTF");
  }

  /* kloug(100, "Syscall %d\n", regs->eax); */
  SWITCH_BEFORE();  /* Save context + kernel paging */
  /* kloug(100, "Context restored\n"); */
  /* u_int32 esp; */
  /* asm volatile ("mov %%esp, %0" : "=r" (esp)); */
  /* kloug(100, "Regs structure at %X and esp at %X\n", regs, 8, esp, 8); */
  /* kloug(100, "Useresp %X ss %x esp %X\n", regs->useresp, 8, regs->ss, regs->esp, 8); */


  syscall(regs->eax);
  /* kloug(100, "Syscall ended\n"); */

  /* Check if the syscall has not ended, and if it is the case select a new process */
  if (should_cycle || state->processes[state->curr_pid].state != Runnable) {
    select_new_process();
    should_cycle = FALSE;
  }

  SWITCH_AFTER();  /* Paging set-up + restore the stack and the context */
}


bool load_code(string program_name, context_t ctx)
{
  /* kloug(100, "Loading %s code\n", program_name); */

  string temp = str_cat("/progs/", program_name);
  string path = str_cat(temp, ".elf");
  mem_free(temp);

  u_int32 inode = find_inode(path, 2);
  if (!inode) {
    return FALSE;
  }
  mem_free(path);

  /* TODO: read until EOF or something */
  inode_t inode_buffer;
  set_inode(inode, &inode_buffer);
  size_t size = inode_buffer.size_low;

  /* Loads ELF file in memory */
  u_int8 *elf_buffer = (u_int8 *)mem_alloc(size);

  u_int8 *current = elf_buffer;
  while (current < elf_buffer + size) {
    current += read_inode_data(inode, current, current - elf_buffer, elf_buffer + size - current);
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

  return TRUE;
}


/**
 * @name switch_to_process - Transfers control to the given new process
 * This ignores the regs structure, as well as the kernel esp
 * @param pid              - The process to transfer control to
 * @return void
 */
void switch_to_process(pid pid)
{
  in_kernel = FALSE;
  should_cycle = FALSE;

  process_t proc = state->processes[pid];

  /* Saves kernel context */
  kernel_context.unallocated_mem  = unallocated_mem;
  kernel_context.first_free_block = first_free_block;

  /* Restores process context */
  context_t ctx = proc.context;
  first_free_block = ctx.first_free_block;
  unallocated_mem  = ctx.unallocated_mem;

  /* Pushes the regs structure on the stack */
  /* kloug(100, "Pushing, kernel ESP %X user ESP %X EIP %X\n",       \ */
        /* kernel_context.esp, 8, ctx.regs->useresp, 8, ctx.regs->eip, 8); */

  /* Simulate an interruption stack frame */
  asm volatile ("push %0" : : "r" (ctx.regs->ss));
  asm volatile ("push %0" : : "r" (ctx.regs->useresp));
  asm volatile ("pushf; mov (%esp), %eax; or $0x200, %eax; mov %eax, (%esp)");
  asm volatile ("push %0" : : "r" (ctx.regs->cs));
  asm volatile ("push %0" : : "r" (ctx.regs->eip));

  /* The user segments */
  asm volatile ("push %0" : : "r" (ctx.regs->ds));
  asm volatile ("push %0" : : "r" (ctx.regs->es));
  asm volatile ("push %0" : : "r" (ctx.regs->fs));
  asm volatile ("push %0" : : "r" (ctx.regs->gs));

  /* /\* Restores process paging *\/ */
  /* u_int32 *code = (u_int32 *)ctx.regs->eip; */
  /* kloug(100, "Oh god\n"); */
  /* log_page_dir(ctx.page_dir); */
  /* kloug(100, "%u %u\n", ctx.page_dir->tables[0x3FF]->pages[0x400 - 16].user, \ */
        /* ctx.page_dir->tables[0x3FF]->pages[0x400 - 16].rw); */

  switch_page_directory(ctx.page_dir);
  /* kloug(100, "Instruction at %X: %X\n", code, 8, *code, 8); */
  /* Let's go! */

  asm volatile ("pop %gs");
  asm volatile ("pop %fs");
  asm volatile ("pop %es");
  asm volatile ("pop %ds");

  /* u_int32 eip; */
  /* asm volatile ("pop %0" : "=r" (eip)); */
  /* writef("%x", eip); */

  asm volatile ("iret;");
}


bool run_executed = FALSE;
bool run_program(string name)
{
  pid pid = 0;
  while (pid < NUM_PROCESSES && state->processes[pid].state != Free) {
    pid++;
  }

  if (pid == NUM_PROCESSES) {
    writef("%frun:%f\tUnable to create a new process\n", LightRed, White);
    return FALSE;
  }

  process_t *proc = &state->processes[pid];
  *proc = new_process(state->curr_pid, 1, TRUE);  /* User processes have a priority of 1 */
  if (!load_code(name, proc->context)) {
    /* Unable to load code */
    writef("%frun:%f\tUnknown file: progs/%s.elf\n", LightRed, White, name);

    mem_free(proc->context.regs);
    free_page_dir(proc->context.page_dir);
    proc->state = Free;

    return FALSE;
  }

  /* kloug(100, "%x %x\n", proc->context.regs->ss, proc->context.regs->cs); */

  enqueue(state->runqueues[1], pid);
  kloug(100, "New process of pid %u (parent %u)\n", pid, state->curr_pid);
  run_executed = TRUE;

  return TRUE;
}


void scheduler_install(bool shell_on)
{
  state = (scheduler_state_t *)mem_alloc(sizeof(scheduler_state_t));
  mem_set(state, 0, sizeof(scheduler_state_t));

  /* Initialization of the state */
  for (priority prio = 0; prio <= MAX_PRIORITY; prio++) {
    state->runqueues[prio] = empty_queue();
  }

  /* Creating idle process */
  pid idle_pid = 0;
  process_t *idle = &state->processes[idle_pid];
  *idle = new_process(idle_pid, 0, TRUE);
  load_code("idle", idle->context);
  enqueue(state->runqueues[0],            idle_pid);

  /* Creating init process */
  pid init_pid = 1;
  process_t *init = &(state->processes[init_pid]);
  *init = new_process(init_pid, MAX_PRIORITY, TRUE);
  load_code("init", init->context);
  enqueue(state->runqueues[MAX_PRIORITY], init_pid);


  if(shell_on) {
    pid shell_pid = 2;
    process_t *shell = &(state->processes[shell_pid]);
    *shell = new_process(init_pid, 1, shell_pid);
    load_code("shell", shell->context);
    enqueue(state->runqueues[1], shell_pid);
  }

  /* /\* Creating timer1 process *\/ */
  /* pid timer1_pid = 2; */
  /* process_t *timer1 = &(state->processes[timer1_pid]); */
  /* *timer1 = new_process(timer1_pid, MAX_PRIORITY-1, TRUE); */
  /* load_code("timer1", timer1->context); */
  /* enqueue(state->runqueues[MAX_PRIORITY], timer1_pid); */

  /* /\* Creating timer2 process *\/ */
  /* pid timer2_pid = 1; */
  /* process_t *timer2 = &(state->processes[timer2_pid]); */
  /* *timer2 = new_process(timer2_pid, MAX_PRIORITY, TRUE); */
  /* load_code("timer2", timer2->context); */
  /* enqueue(state->runqueues[MAX_PRIORITY], timer2_pid); */

  /* Adds handlers for timer and syscall interruptions */
  extern void *timer_phase(int hz);  /* Defined in timer.c */
  timer_phase(SWITCH_FREQ);
  irq_install_handler(0, timer_handler);
  syscall_install();

  /* kloug(100, "Scheduler installed\n"); */

  state->curr_pid = init_pid;  /* We start with the init process */
  switch_to_process(init_pid);
}
