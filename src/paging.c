#include "types.h"
#include "paging.h"
#include "kheap.h"
#include "logging.h"
#include "error.h"
#include "memory.h"
#include "irq.h"

/* Source of everything: http://www.jamesmolloy.co.uk/tutorial_html/6.-Paging.html */


/** Frame allocation */
/*  TODO: Replace this implementation with a stack of free frames (alloc and free in O(1)) */

/* A bitset of frames - used or free. */
u_int32 *frames;
u_int32 nb_frames;

/* Macros used in the bitset algorithms. */
#define INDEX_FROM_BIT(a)  (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

/* Static function to set a bit in the frames bitset. */
static void set_frame(u_int32 frame_addr)
{
  u_int32 frame = frame_addr/0x1000;
  u_int32 idx = INDEX_FROM_BIT(frame);
  u_int32 off = OFFSET_FROM_BIT(frame);
  frames[idx] |= (0x1 << off);
}

/* Static function to clear a bit in the frames bitset. */
static void clear_frame(u_int32 frame_addr)
{
  u_int32 frame = frame_addr/0x1000;
  u_int32 idx = INDEX_FROM_BIT(frame);
  u_int32 off = OFFSET_FROM_BIT(frame);
  frames[idx] &= ~(0x1 << off);
}

/* /\* Static function to test if a bit is set. *\/ */
/* static u_int32 test_frame(u_int32 frame_addr) */
/* { */
/*   u_int32 frame = frame_addr/0x1000; */
/*   u_int32 idx = INDEX_FROM_BIT(frame); */
/*   u_int32 off = OFFSET_FROM_BIT(frame); */
/*   return (frames[idx] & (0x1 << off)); */
/* } */

/* Static function to find the first free frame. */
static u_int32 first_frame()
{
  u_int32 i, j;
  for (i = 0; i < INDEX_FROM_BIT(nb_frames); i++) {
    if (frames[i] != 0xFFFFFFFF) { /* Nothing free, exit early. */
      /* At least one bit is free here. */
      for (j = 0; j < 32; j++) {
        u_int32 toTest = 0x1 << j;
        if ( !(frames[i]&toTest) )
          return i*4*8+j;
      }
    }
  }

  return (u_int32)-1;  /* No free frames */
}



/* Function to allocate a frame. */
void alloc_frame(page_t *page, bool is_kernel, bool is_writeable)
{
  if (page->frame != 0) {
    return;  /* Frame was already allocated, return straight away. */
  } else {
    u_int32 idx = first_frame();    /* idx is now the index of the first free frame. */

    if (idx == (u_int32) (-1)) {
      writef("Début-- %d --Fin\n", (u_int32)idx);
      throw("No free frames!");
    }

    set_frame(idx * 0x1000);        /* This frame is now ours! */
    page->present = TRUE;           /* Mark it as present. */
    page->rw      = is_writeable;   /* Should the page be writable? */
    page->user    = is_kernel;      /* Should the page be user-mode? */
    page->frame   = idx;
  }
}

/* Function to deallocate a frame. */
void free_frame(page_t *page)
{
  u_int32 frame;
  if (!(frame = page->frame)) {
    return;              /* The given page didn't actually have an allocated frame! */
  } else {
    clear_frame(frame);  /* Frame is now free again. */
    page->frame = 0x0;   /* Page now doesn't have a frame. */
  }
}


/** Paging code */

extern u_int32 brk;  /* Defined in kheap.c */

page_directory_t *kernel_directory;
page_directory_t *current_directory;

void paging_install()
{
  /* The size of physical memory. For the moment we
   * assume it is 16MB big. */
  u_int32 mem_end_page = 0x1000000;

  nb_frames = mem_end_page / 0x1000;
  frames = (u_int32*)kmalloc(INDEX_FROM_BIT(nb_frames));
  mem_set(frames, 0, INDEX_FROM_BIT(nb_frames));

  /* Let's make a page directory. */
  kernel_directory = (page_directory_t*)kmalloc_aligned(sizeof(page_directory_t));
  mem_set(kernel_directory, 0, sizeof(page_directory_t));
  current_directory = kernel_directory;

  /* We need to identity map (phys addr = virt addr) from
   * 0x0 to the end of used memory, so we can access this
   * transparently, as if paging wasn't enabled. */
  u_int32 i = 0;
  while (i < mem_end_page) {  /* TODO: do this properly */
    /* Kernel code is readable but not writable from user-space. */
    alloc_frame(get_page(i, 1, kernel_directory), 0, 0);
    i += 0x1000;
  }

  /* Before we enable paging, we must register our page fault handler. */
  irq_install_handler(14, page_fault);

  /* Now, enable paging! */
  switch_page_directory(kernel_directory);
}

void switch_page_directory(page_directory_t *dir)
{
  /* Loads address of the current directory into cr3 */
  current_directory = dir;
  __asm__ __volatile__("mov %0, %%cr3":: "r"(&dir->tables_physical_address));

  /* Reads current cr0 */
  u_int32 cr0;
  __asm__ __volatile__("mov %%cr0, %0": "=r"(cr0));

  /* Enables paging! */
  cr0 |= 0x80000000;
  __asm__ __volatile__("mov %0, %%cr0":: "r"(cr0));
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


void page_fault(regs_t *regs)
{
  /* A page fault has occurred. */
  /* The faulting address is stored in the CR2 register. */
  u_int32 faulting_address;
  __asm__ __volatile__("mov %%cr2, %0" : "=r" (faulting_address));

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
