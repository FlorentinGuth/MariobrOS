#ifndef PAGING_H
#define PAGING_H

/* paging.h:
 * Contains paging-related functions.
 */

#include "types.h"
#include "bitset.h"

/* Reference for paging directory structure: http://valhalla.bofh.pl/~l4mer/WDM/secureread/pde-pte.htm */


/* Whether the paging is enabled */
bool paging_enabled;  /* This must be set to FALSE by kmain before anything */


/* A bitset of frames (physical pages) - used or free */
bitset_t frames;


typedef struct page_table_entry {
  /* All those refer to the page pointed by the address in the page table entry */
  bool    present        :  1;  /* 1 if page present in memory */
  bool    rw             :  1;  /* 0 if read-only (only in user-mode), 1 if read/write */
  bool    user           :  1;  /* 0 if supervisor level (kernel) */
  bool    write_through  :  1;  /* I have no idea what this is */
  bool    cache_disabled :  1;  /* I have no idea what this is */
  bool    accessed       :  1;  /* 1 if the page has been accessed since last refresh */
  bool    dirty          :  1;  /* 1 if the page been written to since last refresh? */
  bool    pta_index      :  1;  /* Page Table Attribute index, I have no idea what this is */
  bool    global_page    :  1;  /* I have no idea what this is */
  u_int8  available      :  3;  /* Available for us! */
  u_int32 address        : 20;  /* Page address (physical address, shifted right 12 bits) */
} __attribute__((packed)) page_table_entry_t;

typedef struct page_table {
  page_table_entry_t pages[1024];
} __attribute__((packed)) page_table_t;


typedef struct page_directory_entry {
  /* All those bits refer to the page containing the page table */
  bool    present        :  1;  /* 1 if page present in memory */
  bool    rw             :  1;  /* 0 if read-only (only in user-mode), 1 if read/write */
  bool    user           :  1;  /* 0 if supervisor level (kernel) */
  bool    write_through  :  1;  /* I have no idea what this is */
  bool    cache_disabled :  1;  /* I have no idea what this is */
  bool    accessed       :  1;  /* 1 if the page has been accessed since last refresh */
  bool    reserved       :  1;  /* Reserved by the processor, set to 0 */
  bool    page_size      :  1;  /* 0 if 4KB, I don't know what size if 1 */
  bool    global_page    :  1;  /* Ignored */
  u_int8  available      :  3;  /* Available for us! */
  u_int32 address        : 20;  /* Page table address (physical address, shifted right 12 bits) */
} __attribute__((packed)) page_directory_entry_t;

typedef struct page_directory
{
  /* Array of pointers to page tables, i.e. contains their virtual addresses */
  page_table_t *tables[1024];

  /* Array of pointers to the page tables above, but gives their *physical*
   * address, for loading into the CR3 register. */
  page_directory_entry_t entries[1024];

  /* The physical address of the entries. This comes into play
   * when we get our kernel heap allocated and the directory
   * may be in a different location in virtual memory. */
  u_int32 physical_address;  /* Unused for now */
} __attribute__((packed)) page_directory_t;


page_directory_t* current_directory;
page_directory_t* kernel_directory;
page_directory_t* base_directory;    /* Copy of the original kernel directory */
u_int32 start_of_user_heap;          /* Constant, set to the address of the user heap start */
u_int32 start_of_user_stack;         /* Constant, set to the address of the user stack start */


/**
 * @name paging_install - Enables paging
 * @return void
 */
void paging_install();


/**
 * @name request_virtual_space - Asks for access to a virtual page
 * @param virtual_address      - An address in the requested page
 * @param is_kernel            - Whether the page should be in kernel mode
 * @param is_writable          - Whether the page should be writable (only applies in user-mode)
 * @return bool                - Whether the request was successful
 */
bool request_virtual_space(u_int32 virtual_address, bool is_kernel, bool is_writable);


/**
 * @name free_virtual_space - Frees up the virtual space, so someone else can access it
 * @param virtual_address   - An address in the freed page
 * @return void
 */
void free_virtual_space(u_int32 virtual_address);


/**
 * @name switch_page_directory - Loads the new page directory into the CR3 register
 * @param new                  -
 * @return void
 */
void switch_page_directory(page_directory_t *new);

/**
 * @name new_page_dir - Allocates and creates a new page directory, with the
 * kernel code and data (including stack) at the same virtual space.
 * The new virtual space also includes a heap (start_of_user_heap) and a stack
 * (start_of_user_stack).
 * @return page_directory_t*
 */
page_directory_t *new_page_dir();


#endif
