#ifndef GDT_H
#define GDT_H

#include "types.h"

/* Defines a GDT entry. We say packed, because it prevents the
 * compiler from doing things that it thinks is best: Prevent
 * compiler "optimization" by packing */
struct gdt_entry
{
  unsigned short limit_low;
  unsigned short base_low;
  unsigned char  base_middle;
  unsigned char  access;
  unsigned char  limit_middle:4;
  unsigned char  other:4;
  unsigned char  base_high;
} __attribute__((packed));

/* Special pointer which includes the limit: The max bytes
 * taken up by the GDT, minus 1. Again, this NEEDS to be packed */
struct gdt_ptr
{
  unsigned short limit;
  unsigned int base;
} __attribute__((packed));


struct tss {
  u_int16 previous_task, previous_task_unused;
  u_int32 esp0;
  u_int16 ss0, ss0_unused;
  u_int32 esp1;
  u_int16 ss1, ss1_unused;
  u_int32 esp2;
  u_int16 ss2, ss2_unused;
  u_int32 cr3;
  u_int32 eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
  u_int16 es, es_unused;
  u_int16 cs, cs_unused;
  u_int16 ss, ss_unused;
  u_int16 ds, ds_unused;
  u_int16 fs, fs_unused;
  u_int16 gs, gs_unused;
  u_int16 ldt_selector, ldt_sel__unused;
  u_int16 debug_flag, io_map;
} __attribute__((packed));


/**
 * @name gdt_install - Sets up the GDT.
 */
void gdt_install();

#endif
