#ifndef GDT_H
#define GDT_H


/** gdt_install
 *  Sets up the GDT
 */
void gdt_install();

/* This will be a function in start.asm. We use this to properly
*  reload the new segment registers */
extern void gdt_flush();

#endif
