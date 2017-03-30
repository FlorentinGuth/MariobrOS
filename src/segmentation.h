/** segmentation.h:
 *  Provides functions for segmentation.
 */

#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#include <stdint.h>

struct gdt_t {
  unsigned int address;
  unsigned short size;
}__attribute__((packed));

typedef struct gdt_entry gdt_e;
struct gdt_entry{
  uint16_t base_0_15;
  uint16_t limit_0_15;
  uint8_t base_24_31;
  uint8_t flags_limits_16_19;
  uint8_t access;
  uint8_t base_16_23;
}__attribute__((packed));


void segmentize(gdt_e GDT[3]);

#endif
