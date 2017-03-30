#include "segmentation.h"
#include "lgdt.h"

/* Source : http://wiki.osdev.org/GDT_Tutorial */

/* Segment selectors : */
/* Bit:     | 15          3 | 2 | 1 0 |
 * Content: | offset (index)| ti| rpl |  
 * Value:   | 0000000000001 | 0 | 0 0 | = 0x08
 * Value:   | 0000000000010 | 0 | 0 0 | = 0x10
 */

void set_null(gdt_e *e)
{
  e->base_0_15 = 0; e->base_16_23 = 0; e->base_24_31 = 0;
  e->limit_0_15 = 0; e->flags_limits_16_19 = 0;
  e->access = 0;
}

void set_exe(gdt_e *e)
{
  /* Bit:     | 7 | 6  5 | 4 | 3 | 2 | 1 | 0 |
   * Content: | Pr| Priv | 1 | Ex| DC| RW| AC|
   * Value:   | 1 | 0  0 | 1 | 1 | 0 | 1 | 0 | = 0x9a */
  e->access = 0x9a;
}

void set_data(gdt_e *e)
{
  /* Bit:     | 7 | 6  5 | 4 | 3 | 2 | 1 | 0 |
   * Content: | Pr| Priv | 1 | Ex| DC| RW| AC|
   * Value:   | 1 | 0  0 | 1 | 0 | 0 | 1 | 0 | = 0x92 */
  e->access = 0x92;
}

void set_full(gdt_e *e)
{
  e->base_0_15 = 0; e->base_16_23 = 0; e->base_24_31 = 0;
  e->limit_0_15 = 0xffff;

  /* Bit:     | 7 | 6 | 5  4 | 3     0 |
   * Type:    |    Flags     |  Limit  |
   * Content: | Gr| Sz| 0  0 |         |  
   * Value:   | 1 | 1 | 0  0 | 1 1 1 1 | = 0xcf */
  e->flags_limits_16_19 = 0xcf;
}

void segmentize(gdtr* gdtr)
{
  set_null(&gdtr->null);
  set_full(&gdtr->code); set_full(&gdtr->data);
  set_exe(&gdtr->code);
  set_data(&gdtr->data);
}

void set_gdt(gdt_l *gdt)
{
  load_gdt(gdt);
  set_segments();
  __asm__ __volatile__("flush_cs:");
}
