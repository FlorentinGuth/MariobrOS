/** segmentation.h:
 *  Provides functions for segmentation.
 */

#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#include <stdint.h>

typedef struct gdt_entry gdt_e;
struct gdt_entry{
  uint16_t base_0_15;
  uint16_t limit_0_15;
  uint8_t base_24_31;
  uint8_t flags_limits_16_19;
  uint8_t access;
  uint8_t base_16_23;
}__attribute__((packed));

typedef struct gdtr gdtr;
struct gdtr{
  gdt_e null;
  gdt_e code;
  gdt_e data;
}__attribute__((packed));

typedef struct gdt_to_load gdt_l;
struct gdt_to_load{
  gdtr *gdtr;
  unsigned short size;
}__attribute__((packed));


void segmentize(gdtr *gdtr);

void set_gdt(gdt_l *gdt);

#endif
