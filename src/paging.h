#ifndef PAGING_H
#define PAGING_H

/* paging.h:
 * Contains paging-related functions.
 */

#include "types.h"


typedef struct page
{
  bool    present    : 1;   /* Page present in memory */
  bool    rw         : 1;   /* Read-only if clear, readwrite if set */
  bool    user       : 1;   /* Supervisor level only if clear */
  bool    accessed   : 1;   /* Has the page been accessed since last refresh? */
  bool    dirty      : 1;   /* Has the page been written to since last refresh? */
  u_int16 unused     : 7;   /* Amalgamation of unused and reserved bits */
  u_int32 frame      : 20;  /* Frame address (shifted right 12 bits) */
} page_t;

typedef struct page_table
{
  page_t pages[1024];
} page_table_t;

typedef struct page_directory
{
  /* Array of pointers to page tables. */
  page_table_t *tables[1024];

  /* Array of pointers to the page tables above, but gives their *physical*
   * location, for loading into the CR3 register. */
  u_int32 tables_physical_address[1024];

  /* The physical address of tables_physical_address. This comes into play
   * when we get our kernel heap allocated and the directory
   * may be in a different location in virtual memory. */
  u_int32 physical_address;
} page_directory_t;


/** paging_install:
 *  Enables paging.
 */
void paging_install();


/** switch_page_directory:
 *  Loads the new page directory into the CR3 register.
 */
void switch_page_directory(page_directory_t *new);

/** get_page:
 *  Retrieves a pointer to the page required.
 *
 *  @param address the address whose we should search in which page it is
 *  @param make    if true, create the page if needed
 *  @param dir     a pointer to the page directory
 *  @return        the page of the given page directory which contains the given address
 */
page_t * get_page(u_int32 address, bool make, page_directory_t *dir);

/** page_fault:
 *  Handler for page faults.
 */
void page_fault(regs_t *regs);

#endif
