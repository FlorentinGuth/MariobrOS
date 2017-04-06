#ifndef PAGING_H
#define PAGING_H


typedef unsigned int page_directory_entry;
typedef unsigned int page_table_entry;

typedef page_table_entry page_table[1024];
typedef page_directory_entry page_directory[1024];


/** paging_install:
 *  Enables paging. For now, this is just stupid identity paging, marking everything as kernel-owned.
 */
void paging_install();

#endif
