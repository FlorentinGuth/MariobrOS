#include "types.h"
#include "paging.h"
#include "kheap.h"
#include "logging.h"
#include "error.h"
#include "memory.h"
#include "irq.h"
#include "malloc.h"
#include "process.h"

/* Source of everything: http://www.jamesmolloy.co.uk/tutorial_html/6.-Paging.html */


#define INDEX_FROM_FRAME(frame) (frame / 0x1000)
#define FRAME_FROM_INDEX(index) (index * 0x1000)


/**
 * @name map_page_to_frame - Maps the given page to the given frame (physical page)
 * @param page             - The page (virtual address)
 * @param frame            - The frame (physical address / 0x1000)
 * @param is_kernel        - Whether the page is in kernel mode
 * @param is_writable      - Whether the page is writable (or read-only)
 * @return void
 */
void map_page_to_frame(page_t *page, u_int32 frame, bool is_kernel, bool is_writable)
{
  /* Marks the physical frame as used, if not already */
  set_bit(frames, frame, TRUE);

  page->present = TRUE;
  page->rw      = is_writable;
  page->user    = is_kernel;
  page->frame   = frame;
}

/**
 * @name map_page     - Maps the given page (virtual address) to a free frame (physical page)
 * @param page        - The page one wants access to
 * @param is_kernel   - Whether the page is in kernel mode
 * @param is_writable - Whether the page is writable (or read-only)
 * @return void
 */
void map_page(page_t *page, bool is_kernel, bool is_writable)
{
  map_page_to_frame(page, first_false_bit(frames), is_kernel, is_writable);
}


/**
 * @name free_page       - Marks the page (virtual space) as non-present anymore
 * @param page           - The page one wants no more access to
 * @param set_frame_free - Whether to mark the assoiated frame (physical space) as free
 * @return void
 */
void free_page(page_t *page, bool set_frame_free)
{
  if (set_frame_free) {
    set_bit(frames, page->frame, FALSE);
  }
  page->frame = 0x0;
}


void paging_install()
{
  /* Set up the frames bitset */
  frames.length = INDEX_FROM_FRAME(UPPER_MEMORY);
  frames.bits = (u_int32 *)kmalloc(frames.length);
  mem_set(kernel_frames, 0, INDEX_FROM_BIT(nb_frames));

  /* Let's make a page directory. */
  kernel_directory = (page_directory_t*)kmalloc_aligned(sizeof(page_directory_t));
  mem_set(kernel_directory, 0, sizeof(page_directory_t));
  current_directory = kernel_directory;

  /* We need to identity map (phys addr = virt addr) from
   * 0x0 to the end of the kernel heap, so we can access this
   * transparently, as if paging wasn't enabled. */
  u_int32 i = 0;
  while (i < END_OF_KERNEL_HEAP) {
    /* Kernel code and data is readable but not writable from user-space. */
    alloc_frame(kernel_frames, get_page(i, TRUE, kernel_directory), TRUE, FALSE);
    i += 0x1000;
  }

  /* Before we enable paging, we must register our page fault handler. */
  isr_install_handler(14, page_fault_handler);

  /* Now, enable paging! */
  switch_page_directory(kernel_directory);
}


void new_page_directory(process_t *proc)
{
  /* Let's make a page directory. */
  page_directory_t *page_dir = (page_directory_t*)mem_alloc_aligned(sizeof(page_directory_t), 0x1000);
  mem_set(page_dir, 0, sizeof(page_directory_t));

  /* We need to identity map (phys addr = virt addr) from
   * 0x0 to the end of the kernel heap, so we can access this
   * transparently, as if paging wasn't enabled. */
  u_int32 i = 0;
  while (i < END_OF_KERNEL_HEAP) {
    /* Kernel code and data is readable but not writable from user-space. */
    alloc_frame(kernel_frames, get_page(i, TRUE, page_dir), TRUE, FALSE);
    i += 0x1000;
  }

  proc->context.page_dir = page_dir;
}


void switch_page_directory(page_directory_t *dir)
{
  /* Loads address of the current directory into cr3 */
  current_directory = dir;
  asm volatile ("mov %0, %%cr3" : : "r"(&dir->tables_physical_address));

  /* Reads current cr0 */
  u_int32 cr0;
  asm volatile ("mov %%cr0, %0" : "=r"(cr0));

  /* Enables paging! */
  cr0 |= 0x80000000;
  asm volatile ("mov %0, %%cr0" : : "r"(cr0));
}

page_t *get_page(u_int32 address, bool make, page_directory_t *dir)
{
  /* Turn the address into an index. */
  address /= 0x1000;

  /* Find the page table containing this address. */
  u_int32 table_idx = address / 1024;

  if (dir->tables[table_idx]) { /* If this table is already assigned */
    return &dir->tables[table_idx]->pages[address%1024];
  }
  else if(make) {
    u_int32 tmp;  /* To hold the physical address of the page */
    dir->tables[table_idx] = (page_table_t*)kmalloc_physical_aligned(sizeof(page_table_t), &tmp);

    mem_set(dir->tables[table_idx], 0, 0x1000);
    dir->tables_physical_address[table_idx] = tmp | 0x7; // PRESENT, RW, US.

    return &dir->tables[table_idx]->pages[address%1024];
  }
  else {
    return 0;
  }
}


void page_fault_handler(regs_t *regs)
{
  /* A page fault has occurred. */
  /* The faulting address is stored in the CR2 register. */
  u_int32 faulting_address;
  asm volatile ("mov %%cr2, %0" : "=r" (faulting_address));

  /* The error code gives us details of what happened. */
  bool present  = !(regs->err_code       & 0x1);  /* Page not present */
  bool rw       =  (regs->err_code >> 1) & 0x1;   /* Write operation? */
  bool us       =  (regs->err_code >> 2) & 0x1;   /* Processor was in user-mode? */
  bool reserved =  (regs->err_code >> 3) & 0x1;   /* Overwritten CPU-reserved bits of page entry? */
  /* bool id       = (regs.err_code >> 4) & 0x1;   /\* Caused by an instruction fetch? *\/ */

  /* Output an error message. */
  write_string("Page fault! ( ");
  if (present)  {write_string("present ");}
  if (rw)       {write_string("read-only ");}
  if (us)       {write_string("user-mode ");}
  if (reserved) {write_string("reserved ");}
  write_string(") at ");
  write_hex(faulting_address);
  write_string("\n");

  throw("Page fault");
}
