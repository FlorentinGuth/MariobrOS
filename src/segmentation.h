/** segmentation.h:
 *  Provides functions for segmentation.
 */

#ifndef SEGMENTATION_H
#define SEGMENTATION_H


struct GDT {
  unsigned int address;
  unsigned short size;
} __atribute__((packed)); /* Prevents GCC from modifying order of attributes */
typedef struct GDT gdt_t;

/** load_gdt:
 *  Wrapper for the ASM instruction 'lgdt'.
 *
 *  @param gdt The GDT to load
 */
void load_gdt(gdt_t gdt);

#endif
