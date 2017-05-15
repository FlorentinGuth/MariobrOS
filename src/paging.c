#include "types.h"
#include "paging.h"
#include "math.h"
#include "logging.h"
#include "error.h"
#include "memory.h"
#include "irq.h"
#include "malloc.h"
#include "process.h"
#include "scheduler.h"

/* Source material: http://www.jamesmolloy.co.uk/tutorial_html/6.-Paging.html */

/* Vocabulary precisions:
 * - a frame is the physical address of a page, divided by 0x1000 (so takes 20 bits)
 *   By extension, it designates the whole range frame - frame + 0x1000
 * - a page is an entry in a page table, it refers to virtual memory
 */

/* TODO:
 * Mark lower memory as usable
 * Do not map page 0
 */

/**
 * @name test_page_dir - Tries switching into the given directory
 * @param dir          -
 * @return void
 */
void test_page_dir(page_directory_t *dir) {
  log_page_dir(current_directory);
  log_page_dir(dir);
  page_directory_t *temp = current_directory;
  switch_page_directory(dir);
  kloug(100, "Everything's fine\n");
  switch_page_directory(temp);
}


#pragma GCC diagnostic ignored "-Wunused-parameter"
void flush_tlb(u_int32 address)
{
  /* asm volatile ("invlpg (%0)" : : "r" (address)); */
  asm volatile ("mov %cr3, %eax; mov %eax, %cr3");
}
#pragma GCC diagnostic pop


u_int32 get_physical_address(page_directory_t *dir, u_int32 virtual_address)
{
  /**
   * Bits  | 31 - 22 (10 bits, i.e. 1024) | 21 - 12 (10 bits, i.e. 1024) | 11 - 0 (12 bits, i.e. 4KB = 0x1000)
   * Usage | offset in the page directory | offset in the page table     | offset in the page
   */
  u_int32 frame_address = virtual_address / 0x1000;
  u_int32 page_index    = frame_address % 1024;
  u_int32 table_index   = frame_address / 1024;

  if (dir->entries[table_index].present) {
    page_table_entry_t page = dir->tables[table_index]->pages[page_index];
    if (page.present) {
      /* The page is present, everything is alright */
      u_int32 physical_address = 0x1000 * page.address + virtual_address % 0x1000;
      /* kloug(100, "Physical address of %X is %X\n", virtual_address, 8, physical_address, 8); */
      return physical_address;
    }
  }

  /* Either the page table or the page is not present, reading at the
   * supplied address would page_fault
   */
  /* kloug(100, "get_physical_address of %X returned NULL\n", virtual_address, 8); */
  return NULL;
}


/**
 * @name map_page_to_frame - Maps the given page to the given frame (physical page)
 * @param page             - The page (virtual address)
 * @param frame            - The frame (physical address / 0x1000)
 * @param is_kernel        - Whether the page is in kernel mode
 * @param is_writable      - Whether the page is writable (or read-only)
 * @return void
 */
void map_page_to_frame(page_table_entry_t *page, u_int32 frame, bool is_kernel, bool is_writable)
{
  /* kloug(100, "Maps page %x to frame %x\n", page, frame); */

  if (frame == 0 || frame >= 0x100000) {
    /* We're referencing physical space beyond 0x1000 * 0x100000 = 0xFFFFFFFF + 1 = 4GB */
    writef("Invalid frame: %x", frame);
    throw("Invalid frame");
  }

  if (page->present) {
    throw("Page already mapped");
  }

  /* Marks the physical frame as used, if not already */
  set_bit(frames, frame, TRUE);  /* TODO: reference counting? */

  page->present = TRUE;
  page->rw      = is_writable;
  page->user    = !is_kernel;
  page->address = frame;

  flush_tlb((u_int32)page);
}

/**
 * @name map_page     - Maps the given page (virtual address) to a free frame (physical page)
 * @param page        - The page one wants access to
 * @param is_kernel   - Whether the page is in kernel mode
 * @param is_writable - Whether the page is writable (only in user-mode)
 * @return bool       - Whether the mapping was successful
 */
bool map_page(page_table_entry_t *page, bool is_kernel, bool is_writable)
{
  u_int32 frame = first_false_bit(frames);
  if (frame == (u_int32)(-1)) {
    /* No more free frames! */
    kloug(100, "No more free frames\n");
    return FALSE;
  } else {
    map_page_to_frame(page, frame, is_kernel, is_writable);
    return TRUE;
  }
}


/**
 * @name free_page       - Marks the page (virtual space) as non-present anymore
 * @param page           - The page one wants no more access to
 * @param set_frame_free - Whether to mark the associated frame (physical space) as free
 * @return void
 */
void free_page(page_table_entry_t *page, bool set_frame_free)
{
  if (set_frame_free) {
    set_bit(frames, page->address, FALSE);
  }
  page->present = FALSE;

  /* Flush the TLB entry relevant to the page */
  /* asm volatile ("invlpg (%0)" : : "r" (page->address * 0x1000)); */
  /* Flush the entire TLB, because it works better (and only god knows why) */
  flush_tlb((u_int32)page);
}


/**
 * @name make_page_table - Allocates and creates a page table
 * @param dir            - Paging directory
 * @param table_index    - Index of the associated entry in the page directory
 * @param is_kernel      - Whether the page which will contain the page table should be in kernel mode
 * @param is_writable    - Whether the page which will contain the page table should be writable
 * @return void
 */
void make_page_table(page_directory_t *dir, u_int32 table_index, bool is_kernel, bool is_writable)
{
  /* kloug(100, "Making page table %x\n", table_index); */

  if (dir->tables[table_index]) {
    /* Page table already made */
    throw("Page table already made");
  }

  /* Allocates space for the page table, which must be page-aligned */
  u_int32 page_table_address = (u_int32)mem_alloc_aligned(sizeof(page_table_t), 0x1000);
  u_int32 physical_address;
  if (paging_enabled) {
    physical_address = get_physical_address(current_directory, page_table_address);
  } else {
    physical_address = page_table_address;  /* Will be identity mapped anyway */
  }

  /* Set-up the table by setting the whole page to 0 */
  mem_set((void *)page_table_address, 0, 0x1000);
  dir->tables[table_index] = (page_table_t *)page_table_address;

  /* Set-up the page directory entry */
  page_directory_entry_t *entry = &dir->entries[table_index];
  entry->present   = TRUE;
  entry->rw        = is_writable;
  entry->user      = !is_kernel;
  entry->page_size = FALSE;        /* Should already be 0, but ensures 4KB size */
  entry->address   = physical_address / 0x1000;

  flush_tlb((u_int32)dir->tables[table_index]);
}


/**
 * @name  get_page - Retrieves a pointer to the page entry corresponding to the given address
 * @param dir      - A pointer to the page directory
 * @param address  - The (virtual) address whose we should search in which page it is
 * @return           The page of the given page directory which contains the given address
 */
page_table_entry_t *get_page(page_directory_t *dir, u_int32 address, bool is_kernel, bool is_writable)
{
  /**
   * Bits  | 31 - 22 (10 bits, i.e. 1024) | 21 - 12 (10 bits, i.e. 1024) | 11 - 0 (12 bits, i.e. 0x1000)
   * Usage | offset in the page directory |   offset in the page table   |      offset in the page
   */
  u_int32 frame_address = address / 0x1000;
  u_int32 page_index    = frame_address % 1024;
  u_int32 table_index   = frame_address / 1024;

  page_table_t *page_table = dir->tables[table_index];
  if (!dir->entries[table_index].present) {
    /* kloug(100, "get_page makes a page table\n"); */
    make_page_table(dir, table_index, is_kernel, is_writable);  /* A page table is kernel-only */
    page_table = dir->tables[table_index];
  }

  return &page_table->pages[page_index];
}


/**
 * @name find_unmapped_page - Finds the first unmapped page
 * This might lead to the creation of a page table
 * @param dir               - The page directory
 * @return                  - The index (1024 * index_table + index_page) of a free page
 */
u_int32 find_unmapped_page(page_directory_t *dir)
{
  /* Start at second page, because physical_address(0) = 0... */
  for (u_int32 page = 1; page < 1024 * 1024; page++) {
    if (!get_physical_address(dir, 0x1000 * page)) {
      return page;
    }
  }

  /* All pages are mapped */
  kloug(100, "find_unmapped_page returned NULL\n");
  return NULL;
}



bool request_virtual_space(page_directory_t *dir, u_int32 virtual_address, bool is_kernel, bool is_writable)
{
  /* kloug(100, "Virtual space at %X requested\n", virtual_address, 8); */

  page_table_entry_t *page = get_page(dir, virtual_address, is_kernel, is_writable);

  if (page->present) {
    kloug(100, "Virtual space is already used by someone else\n");
    return FALSE;
  }

  return map_page(page, is_kernel, is_writable);
}

u_int32 request_physical_space(page_directory_t *dir, u_int32 physical_address, \
                               bool is_kernel, bool is_writable)
{
  /* kloug(100, "Physical space at %X requested\n", physical_address, 8); */

  u_int32 index = find_unmapped_page(dir);
  if (!index) {
    kloug(100, "All pages are mapped\n");
    return NULL;
  }

  u_int32 virtual_address = index * 0x1000 + physical_address % 0x1000;
  page_table_entry_t *page = get_page(dir, virtual_address, is_kernel, is_writable);
  map_page_to_frame(page, physical_address / 0x1000, is_kernel, is_writable);

  /* kloug(100, "Mapped from %X\n", virtual_address, 8); */
  return virtual_address;
}

void free_virtual_space(page_directory_t *dir, u_int32 virtual_address, bool free_frame)
{
  page_table_entry_t *page = get_page(dir, virtual_address, TRUE, FALSE);
  /* FIXME: crash if has to make page table */

  free_page(page, free_frame);
}


void switch_page_directory(page_directory_t *dir)
{
  /* kloug(100, "Switching page directory to the one at %X\n", dir->physical_address, 8); */
  /* kloug(100, "dir %X phys %X entries %X\n", dir, 8, dir->physical_address, 8, dir->entries, 8); */

  /* Disables paging */
  u_int32 cr0;
  asm volatile ("mov %%cr0, %0" : "=r" (cr0));
  /* cr0 &= ~0x80000000; */
  /* asm volatile ("mov %0, %%cr0" : : "r" (cr0)); */

  /* Loads address of the current directory into cr3 */
  current_directory = dir;
  u_int32 to_write = dir->physical_address;
  /* kloug(100, "Before writing %X to cr3\n", to_write, 8); */
  asm volatile ("mov %0, %%cr3"  : : "r" (to_write));
  u_int32 cr3;
  asm volatile ("mov %%cr3, %0" : "=r" (cr3));
  /* kloug(100, "Wrote %X to cr3\n", cr3, 8); */

  /* Enables paging! */
  cr0 |= 0x80000000;
  /* kloug(100, "Before writing to cr0\n"); */
  asm volatile ("mov %0, %%cr0" : : "r" (cr0));
  /* kloug(100, "Wrote to cr0\n"); */
}


extern scheduler_state_t *state;
void page_fault_handler(regs_t *regs)
{
  /* The faulting address is stored in the CR2 register. */
  u_int32 faulting_address;
  asm volatile ("mov %%cr2, %0" : "=r" (faulting_address));
  asm volatile ("mov %0,   %%edi" : : "r" (regs->err_code));

  /* The error code gives us details of what happened. */
  bool present  = (regs->err_code       & 0x1);  /* Whether the page is present */
  bool rw       = (regs->err_code >> 1) & 0x1;   /* Write operation? */
  bool us       = (regs->err_code >> 2) & 0x1;   /* Processor was in user-mode? */
  bool reserved = (regs->err_code >> 3) & 0x1;   /* Overwritten CPU-reserved bits of page entry? */
  bool id       = (regs->err_code >> 4) & 0x1;   /* Caused by an instruction fetch? */

  /** Source: http://wiki.osdev.org/Paging
   *  US RW  P - Description
   *   0  0  0 - Supervisory process tried to read a non-present page entry
   *   0  0  1 - Supervisory process tried to read a page and caused a protection fault
   *   0  1  0 - Supervisory process tried to write to a non-present page entry
   *   0  1  1 - Supervisory process tried to write a page and caused a protection fault
   *   1  0  0 - User process tried to read a non-present page entry
   *   1  0  1 - User process tried to read a page and caused a protection fault
   *   1  1  0 - User process tried to write to a non-present page entry
   *   1  1  1 - User process tried to write a page and caused a protection fault
   */

  /* Temporary, TODO remove */
  writef("Page fault at %x, present %u r %u user-mode %u reserved %u instruction fetch %u", \
         faulting_address, present, rw, us, reserved, id);
  /* writef("Err_code: %x\n", regs->err_code); */
  /* page_table_entry_t *page = get_page(state->processes[state->curr_pid].context.page_dir, faulting_address); */
  /* writef("Entry: %x\n", *page); */
  writef("cs %x ss %x ds %x eip %x esp %x\n", regs->cs, regs->ss, regs->ds, regs->eip, regs->useresp);
  throw("PAGE_FAULT");

  if (!present) {
    /* The page was not present, let's try to make it! */
    /* TODO: apply this only if is the next page of the stack, because malloc should
     * handle paging himself */

    /* TODO detect if kernel mode, but the kernel should not page-fault anyway */
    if (request_virtual_space(current_directory, faulting_address, FALSE, TRUE)) {
      /* Let's return to the faulting code */
      return;
    }

    /* The request failed, there's no more memory or the space is used by someone else */
  } else {
    /* There's nothing we can do for you */
    writef("Page fault at %x, present %u rw %u user-mode %u reserved %u instruction fetch %u", \
           faulting_address, present, rw, us, reserved, id);
    throw("PAGE_FAULT");
  }
}


/**
 * @name clone_page_table - Copy a page table, linking the pages
 * @param table           - The page table to copy
 * @return page_table_t*
 */
page_table_t *clone_page_table(page_table_t *table)
{
  page_table_t *copy = (page_table_t *)mem_alloc_aligned(sizeof(page_table_t), 0x1000);
  mem_copy(copy, table, sizeof(page_table_t));
  /* kloug(100, "Cloned page table at (virtual) %X to %X\n", table, 8, copy, 8); */
  return copy;
}

/**
 * @name clone_directory - Creates an identical page directory, with new page tables (but links pages)
 * @param dir            - The page directory to copy
 * @return page_directory_t*
 */
page_directory_t *clone_directory(page_directory_t *dir)
{
  /* kloug(100, "Cloning directory\n"); */
  page_directory_t *copy = (page_directory_t *)mem_alloc_aligned(sizeof(page_directory_t), 0x1000);
  mem_set(copy, 0, sizeof(page_directory_t));

  copy->physical_address = get_physical_address(current_directory, (u_int32)copy);

  /* Now, iterate the copy until both directories are equal */
  bool equal;
  do {
    equal = TRUE;
    /* Search the whole directories for a difference */
    for (u_int32 table_index = 0; table_index < 1024; table_index++) {
      if (dir->entries[table_index].present) {
        if (!copy->entries[table_index].present) {
          /* Add the page table */
          copy->tables[table_index] = clone_page_table(dir->tables[table_index]);
          /* Copy the entry as well, the address will be updated at the end */
          copy->entries[table_index] = dir->entries[table_index];

          equal = FALSE;
        }
        /* Since we link the pages to the same frame, there's nothing else to do! */
      }
    }
  } while (!equal);

  /* kloug(100, "Infernal cloning loop ended\n"); */

  /* Now the physical addresses, once everything is mapped */
  for (int index = 0; index < 1024; index++) {
    if (dir->entries[index].present) {
      copy->entries[index].address = \
        get_physical_address(current_directory, (u_int32)copy->tables[index]) / 0x1000;
    }
  }

  /* test_page_dir(copy); */
  return copy;
}


u_int32 START_OF_USER_STACK, START_OF_USER_HEAP, START_OF_USER_CODE;
/**
 * @name set_user_addresses - Set up START_OF_USER addresses, base on base_dir
 * @return void
 */
void set_user_addresses()
{
  /* The code is linked at the higher end of virtual memory */
  START_OF_USER_CODE = 0xFFFF0000;               /* 64KB is enough */
  /* The stack starts at the same address, going downward */
  START_OF_USER_STACK = START_OF_USER_CODE - 4;

  /* We search for the first unmapped page in memory */
  u_int32 address = 0x1000;
  while (get_physical_address(base_directory, address)) {
    address += 0x1000;
  }
  START_OF_USER_HEAP = address;

  kloug(100, "Start of user heap: %X, stack: %X, code: %X\n", \
        START_OF_USER_HEAP, 8, START_OF_USER_STACK, 8, START_OF_USER_CODE, 8);
}


void paging_install()
{
  /* Before we enable paging, we must register our page fault handler
   * We do this early for debugging purposes...
   */
  isr_install_handler(14, page_fault_handler);

  /* Set up the frames bitset */
  frames = empty_bitset(floor_ratio(UPPER_MEMORY, 0x1000));
  set_bit(frames, 0, TRUE);
  /* We use floor_ratio instead of ceil_ratio to be sure to have only full pages,
   * rather than an incomplete one at the upper end of memory.
   */

  /* Let's make a page directory */
  kernel_directory = (page_directory_t *)mem_alloc_aligned(sizeof(page_directory_t), 0x1000);
  mem_set(kernel_directory, 0, sizeof(page_directory_t));
  kernel_directory->physical_address = (u_int32)kernel_directory;  /* Will be identity paged */

  current_directory = kernel_directory;

  /* We'll see, if problems arise, uncomment the following lines */
  /* /\* First: allocate every page tables to avoid further problems *\/ */
  /* u_int32 nb_pages  = UPPER_MEMORY / 0x1000; */
  /* u_int32 nb_tables = ceil_ratio(nb_pages, 1024); */
  /* page_table_t* tables = (page_table_t *)mem_alloc_aligned(nb_tables * sizeof(page_table_t), 0x1000); */
  /* mem_set(tables, 0, nb_tables * sizeof(page_table_t)); */

  /* /\* Let's tell the directory about the tables *\/ */
  /* for (u_int32 table_index = 0; table_index < nb_tables; table_index++) { */
  /*   kernel_directory->tables[table_index]  = &tables[table_index]; */

  /*   page_directory_entry_t *entry = &kernel_directory->entries[table_index]; */
  /*   entry->present   = TRUE; */
  /*   entry->rw        = FALSE; */
  /*   entry->user      = TRUE; */
  /*   entry->page_size = FALSE;        /\* Should already be 0, but ensures 4KB size *\/ */
  /*   entry->address = (u_int32)(&tables[table_index]) / 0x1000; */
  /* } */

  /* We need to identity map (phys addr = virt addr) from 0x0 to the end of the
   * kernel heap (given by mem_alloc), so we can access this transparently, as
   * if paging wasn't enabled. Note that the heap can grow during the loop turns,
   * as we will allocate place for the page tables.
   */
  for (u_int32 frame = 0x1000; frame < (u_int32)unallocated_mem; frame += 0x1000) {
    /* Kernel code and data is readable but not writable from user-space */
    /* kloug(100, "Identity-mapping frame %x\n", frame); */
    map_page_to_frame(get_page(kernel_directory, frame, TRUE, FALSE), frame / 0x1000, TRUE, FALSE);
  }

  /* Now, enable paging! */
  switch_page_directory(kernel_directory);
  paging_enabled = TRUE;

  /* Let's keep a copy of the original kernel directory for later use */
  base_directory = clone_directory(kernel_directory);
  set_user_addresses();

  /* kloug(100, "Paging installed\n"); */
}


page_directory_t *new_page_dir(void **user_first_free_block, void **user_unallocated_mem)
{
  /* kloug(100, "New page dir\n"); */

  page_directory_t *new = clone_directory(base_directory);

  /* Add pages for the code, until the end of virtual memory (address will loop to 0) */
  for (u_int32 address = START_OF_USER_CODE; address != 0; address += 0x1000) {
    if (!request_virtual_space(new, address, FALSE, TRUE)) {
      throw("Unable to add user code!");
      /* TODO: free and return NULL */
    }
  }

  /* Add one page of user stack */
  if (!request_virtual_space(new, START_OF_USER_STACK, FALSE, TRUE)) {
    throw("Unable to add user stack!");
    /* TODO: free and return NULL */
  }

  /* And one page of user heap */
  if (!request_virtual_space(new, START_OF_USER_HEAP, FALSE, TRUE)) {
    throw("Unable to add user heap!");
    /* TODO: free and return NULL */
  }

  /* Adding malloc: we need to map this user heap page */
  u_int32 heap_physical = get_physical_address(new, START_OF_USER_HEAP);
  /* kloug(100, "Start of user heap is %X, mapped at physical %X\n", \
     START_OF_USER_HEAP, 8, heap_physical, 8); */
  u_int32 heap_virtual = request_physical_space(current_directory, heap_physical, TRUE, FALSE);

  malloc_new_state(heap_virtual, user_first_free_block, user_unallocated_mem);
  /* Setting the user addresses, inaccurate because of relocation */
  *user_first_free_block = (void *)START_OF_USER_HEAP;
  *user_unallocated_mem  = (void *)START_OF_USER_HEAP + 0x1000;

  free_virtual_space(current_directory, heap_virtual, FALSE);  /* The frame is used by the process */

  /* kloug(100, "New page dir successfully created\n"); */
  return new;
}

page_directory_t *fork_page_dir(page_directory_t *dir)
{
  kloug(100, "Forking directory\n");
  page_directory_t *fork = (page_directory_t *)mem_alloc_aligned(sizeof(page_directory_t), 0x1000);
  if (!fork) {
    return NULL;
  }
  mem_copy(fork, dir, sizeof(page_directory_t));

#define RET_NULL() { kloug(100, "Fork failed\n"); free_page_dir(fork); return NULL; }

  fork->physical_address = get_physical_address(current_directory, (u_int32)fork);

  for (u_int32 table_index = 0; table_index < 1024; table_index++) {
    if (dir->entries[table_index].present) {
      page_table_t *table = dir->tables[table_index];

      /* Make the page table */
      page_table_t *table_fork = (page_table_t *)mem_alloc_aligned(sizeof(page_table_t), 0x1000);
      if (!table_fork) {
        RET_NULL();
      }
      mem_copy(table_fork, table, sizeof(page_table_t));

      for (u_int32 page_index = 0; page_index < 1024; page_index++) {
        if (table->pages[page_index].present) {
          u_int32 virtual = table_index*1024*0x1000 + page_index*0x1000;
          if (get_physical_address(base_directory, virtual)) {
            /* The page is present in base directory: link! */
          } else {
            /* Copy the page */
            if (!request_virtual_space(fork, virtual, FALSE, TRUE)) {
              RET_NULL();
            }

            u_int32 dest = request_physical_space(current_directory, \
                                                  get_physical_address(fork, virtual), TRUE, FALSE);
            if (!dest) {
              RET_NULL();
            }
            u_int32 src  = request_physical_space(current_directory, \
                                                  get_physical_address(dir,  virtual), TRUE, FALSE);
            if (!src) {
              free_virtual_space(current_directory, dest, FALSE);
              RET_NULL();
            }

            mem_copy((void *)dest, (void *)src, 0x1000);

            free_virtual_space(current_directory, src,  FALSE);  /* Is used by the parent */
            free_virtual_space(current_directory, dest, FALSE);  /* Will be used by the child */

            /* Finally, let's set the physical address */
            table_fork->pages[page_index].address = get_physical_address(fork, virtual);
          }
        }
      }

      fork->entries[table_index].address = get_physical_address(current_directory, (u_int32)table_fork);
      fork->tables[table_index] = table_fork;
    }
  }

  kloug(100, "Let's test the forked page dir\n");
  test_page_dir(fork);
  return fork;
}

void free_page_dir(page_directory_t *dir)
{
  /* kloug(100, "Freeing page dir\n"); */

  for (int table_index = 0; table_index < 1024; table_index++) {
    if (dir->entries[table_index].present) {
      for (int page_index = 0; page_index < 1024; page_index++) {
        u_int32 virtual = 0x1000 * (page_index + 1024 * table_index);
        if (!get_physical_address(base_directory, virtual)) {
          /* Page not in base directory: free frame! */
          free_virtual_space(dir, virtual, TRUE);
        }
      }
      mem_free(dir->tables[table_index]);
    }
  }
  mem_free(dir);
}


void log_page_dir(page_directory_t *dir)
{
  kloug(100, "  Logging page dir stored at %X (virtual %X)\n", \
        dir->physical_address, 8, dir, 8);

  for (u_int32 table_index = 0; table_index < 1024; table_index++) {
    if (dir->entries[table_index].present) {
      kloug(100, "  Page table %X stored at %X (virtual %X)\n", \
            table_index, 3, dir->entries[table_index].address*0x1000, 8, \
            dir->tables[table_index], 8);
    }
  }

  u_int32 start_of_virtual_block = 0;
  bool mapped = dir->entries[0].present && dir->tables[0]->pages[0].present;
  u_int32 start_of_physical_block = get_physical_address(dir, 0);
  u_int32 virtual_page = 1;

#define FLUSH_MAPPED() {                                                \
    kloug(100, "  %X-%X mapped to %X-%X\n",                             \
          start_of_virtual_block, 8,                                    \
          virtual_page*0x1000 - 1, 8,                                   \
          start_of_physical_block, 8,                                   \
          start_of_physical_block + virtual_page*0x1000 - start_of_virtual_block - 1, 8); }
#define FLUSH_UNMAPPED() {                      \
    kloug(100, "  %X-%X mapped to NULL\n",      \
          start_of_virtual_block, 8,            \
          virtual_page*0x1000 - 1, 8); }

  while (virtual_page < 1024*1024) {
    u_int32 physical_address = get_physical_address(dir, virtual_page*0x1000);
    if (mapped) {
      if (!physical_address) {
        FLUSH_MAPPED();
        mapped = FALSE;
        start_of_virtual_block = virtual_page*0x1000;
      } else if(physical_address != start_of_physical_block + virtual_page*0x1000 - start_of_virtual_block) {
        FLUSH_MAPPED();
        start_of_virtual_block = virtual_page*0x1000;
        start_of_physical_block = physical_address;
      }
    } else {
      if (physical_address) {
        FLUSH_UNMAPPED();
        start_of_virtual_block = virtual_page*0x1000;
        start_of_physical_block = physical_address;
        mapped = TRUE;
      }
    }
    virtual_page++;
  }

  if (mapped) {
    FLUSH_MAPPED();
  } else {
    FLUSH_UNMAPPED();
  }
}
