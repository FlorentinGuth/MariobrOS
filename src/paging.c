#include "types.h"
#include "paging.h"
#include "kheap.h"
#include "paging_asm.h"


/** make_page_table_entry:
 *  Compress all the given parameters into an entry.
 *
 *  @param page_address The physical address of the page, must be 4KB-aligned (last 12 bits = 0)
 *  @param present      Whether the page is present in memory (i.e. accessible)
 *  @param writeable    If false, the page is read-only, but does not apply in kernel-mode
 *  @param user_mode    If true, this is a user-mode page (user-mode code cannot write in kernel-mode pages)
 *  @return             The page table entry corresponding to the given parameters
 */
page_table_entry make_page_table_entry(unsigned int page_address, bool present, bool writeable, bool user_mode)
{
  /* Source (among others): http://www.jamesmolloy.co.uk/tutorial_html/6.-Paging.html
   * Bits    | 31         12 | 11 10 9 | 8  7 | 6 | 5 | 4  3 |  2  |  1  | 0 |
   * Content | Frame address |  AVAIL  | RSVD | D | A | RSVD | USR | R/W | P |
   */

  page_table_entry entry = page_address & 0xFFF0000;  /* Better safe than sorry */

  entry |=  present   & 0x0001;
  entry |= (writeable & 0x0001) << 1;
  entry |= (user_mode & 0x0001) << 2;

  return entry;
}


/** make_page_directory_entry:
 *  Same as above.
 */
page_directory_entry make_page_directory_entry(unsigned int page_table_address, bool present, bool writeable, bool user_mode)
{
  return (page_directory_entry)make_page_table_entry(page_table_address, present, writeable, user_mode);
}


unsigned int PAGE_DIRECTORY_LOCATION = 0;
void paging_install()
{
  page_directory_entry *pdir = (page_directory_entry *)kmalloc_aligned(sizeof(page_directory));
  page_table_entry     *pt   = (page_table_entry *)PAGE_TABLES_LOCATION;
  unsigned int page_address = 0;

  for (int i = 0; i < 1024; i++) {
    for (int j = 0; j < 1024; j++) {
      page_table_entry table_entry = make_page_table_entry(page_address, TRUE, TRUE, FALSE);
      pt[j] = table_entry;

      page_address += 4 * 1024; /* Adds 4KB, i.e. the size of a page */
    }

    page_directory_entry directory_entry = make_page_directory_entry((unsigned int)pt, TRUE, TRUE, FALSE);
    pdir[i] = directory_entry;

    pt += 4 * 1024;  /* Adds 4KB, i.e. the size of a full page table */
  }

  PAGE_DIRECTORY_LOCATION = (unsigned int)pdir;
  enable_paging();
}
