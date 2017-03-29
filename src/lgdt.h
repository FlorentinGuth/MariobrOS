/** lgdt.h:
 *  Simple wrapper for assembly primitive 'lgdt'.
 */

#ifndef LGDT_H
#define LGDT_H

typedef struct gdt gdt_t
struct gdt {
  unsigned int address;
  unsigned short size;
} __attribute__((packed));


/** load_gdt:
 *  Wrapper for ASM instruction 'lgdt'
 *
 * @param gdt The GDT to load
 */
void load_gdt(gdt_t gdt);

#endif
