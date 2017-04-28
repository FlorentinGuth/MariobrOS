#ifndef GDT_H
#define GDT_H

#include "gdt_asm.h"
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
  unsigned char  granularity;
  unsigned char  base_high;
} __attribute__((packed));

/* Special pointer which includes the limit: The max bytes
 * taken up by the GDT, minus 1. Again, this NEEDS to be packed */
struct gdt_ptr
{
  unsigned short limit;
  unsigned int base;
} __attribute__((packed));

/* TSS structure */
struct tss {
	u_int16 previous_task, __previous_task_unused;
	u_int32 esp0;
    u_int16 ss0, __ss0_unused;
	u_int32 esp1;
	u_int16 ss1, __ss1_unused;
	u_int32 esp2;
	u_int16 ss2, __ss2_unused;
	u_int32 cr3;
	u_int32 eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	u_int16 es, __es_unused;
	u_int16 cs, __cs_unused;
	u_int16 ss, __ss_unused;
	u_int16 ds, __ds_unused;
	u_int16 fs, __fs_unused;
	u_int16 gs, __gs_unused;
	u_int16 ldt_selector, __ldt_sel_unused;
	u_int16 debug_flag, io_map;
} __attribute__ ((packed));

/**
 *  @name gdt_install - Sets up the GDT.
 */
void gdt_install();

void init_pic();

#endif
