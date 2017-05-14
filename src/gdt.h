#ifndef GDT_H
#define GDT_H

#include "gdt_asm.h"
#include "types.h"


u_int32 NULL_SEGMENT;
u_int32 KERNEL_CODE_SEGMENT;
u_int32 KERNEL_DATA_SEGMENT;
u_int32 KERNEL_STACK_SEGMENT;
u_int32 USER_CODE_SEGMENT;
u_int32 USER_DATA_SEGMENT;
u_int32 USER_STACK_SEGMENT;
struct tss TSS_ENTRY;


/* Defines a GDT entry. We say packed, because it prevents the
 * compiler from doing things that it thinks is best: Prevent
 * compiler "optimization" by packing */
typedef struct gdt_entry
{
  u_int16 limit_low   : 16;
  u_int16 base_low    : 16;
  u_int8  base_middle :  8;
  bool    access      :  1;  /* Set to 1 by the processor when the segment is accessed */
  bool    read_write  :  1;  /* Readable bit for code sector (write never allowed),
                                Writable bit for data sector (read always allowed) */
  bool    dir_conform :  1;  /* If data sector: direction,  0 for growing up and 1 for growing down
                              * If code sector: conforming, 0 for executable only by kernel + triggers CPL change */
  bool    executable  :  1;  /* 1 for code sector, 0 for data sector */
  bool    always_one  :  1;  /* Always set to 1 */
  bool    privilege   :  2;  /* 0 for kernel, 3 for user */
  bool    present     :  1;  /* Must be 1 for all valid sectors */
  u_int8  limit_high  :  4;
  bool    available   :  1;
  bool    always_zero :  1;  /* Always set to zero */
  bool    size        :  1;  /* 0 for 16 bits, 1 for 32 bits */
  bool    granularity :  1;  /* If 0 the limit is in bytes, if 1 the limit is in 4KB (page granularity) */
  u_int8  base_high   :  8;
} __attribute__((packed)) gdt_entry_t;

/* Special pointer which includes the limit: The max bytes
 * taken up by the GDT, minus 1. Again, this NEEDS to be packed */
typedef struct gdt_ptr
{
  unsigned short limit;
  unsigned int base;
} __attribute__((packed)) gdt_ptr_t;

/* TSS structure */
typedef struct tss {
  u_int32 previous_tss;
  u_int32 esp0;
  u_int32 ss0;
  u_int32 esp1;
  u_int32 ss1;
  u_int32 esp2;
  u_int32 ss2;
  u_int32 cr3;
  u_int32 eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
  u_int32 es;
  u_int32 cs;
  u_int32 ss;
  u_int32 ds;
  u_int32 fs;
  u_int32 gs;
  u_int32 ldt_selector;
  u_int16 debug_flag, io_map;
} __attribute__ ((packed)) tss_t;

/**
 *  @name gdt_install - Sets up the GDT.
 */
void gdt_install();

void init_pic();


#endif
