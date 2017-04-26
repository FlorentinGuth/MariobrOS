#include "gdt.h"

/* Source : http://www.osdever.net/bkerndev/Docs/gdt.htm */

struct tss default_tss;
struct gdt_entry gdt[GDT_SIZE];
struct gdt_ptr gp;

/* Setup a descriptor in the Global Descriptor Table */
void gdt_set_gate(int num, unsigned int base, unsigned int limit, unsigned char access, unsigned char other)
{
  /* Setup the descriptor base address */
  gdt[num].base_low = base & 0xFFFF;
  gdt[num].base_middle = (base >> 16) & 0xFF;
  gdt[num].base_high = (base >> 24) & 0xFF;

  /* Setup the descriptor limits */
  gdt[num].limit_low = (limit & 0xFFFF);
  gdt[num].limit_middle = (limit >> 16) & 0xF;
  

  /* Finally, set up the granularity and access flags */
  gdt[num].other = other & 0xF;
  gdt[num].access = access;
}


/* Should be called by main. This will setup the special GDT
 * pointer, set up the first 3 entries in our GDT, and then
 * finally call gdt_flush() in our assembler file in order
 * to tell the processor where the new GDT is and update the
 * new segment registers */
void gdt_install()
{

  default_tss.debug_flag = 0x00;
  default_tss.io_map = 0x00;
  default_tss.esp0 = 0x1FFF0;
  default_tss.ss0 = 0x18;
  
  /* Setup the GDT pointer and limit */
  gp.limit = 8 * GDT_SIZE;
  gp.base = GDT_BASE;

  /* Our NULL descriptor */
  gdt_set_gate(0, 0, 0, 0, 0);

  /* The second entry is our Code Segment. The base address
   * is 0, the limit is 4GBytes, it uses 4KByte granularity,
   * uses 32-bit opcodes, and is a Code Segment descriptor. */
  gdt_set_gate(1, 0, 0xFFFFF, 0x9B, 0x0D);

  /* The third entry is our Data Segment. It's EXACTLY the
   * same as our code segment, but the descriptor type in
   * this entry's access byte says it's a Data Segment */
  gdt_set_gate(2, 0, 0xFFFFF, 0x93, 0x0D);

  gdt_set_gate(3, 0, 0x00000, 0x97, 0x0D); // stack?

  gdt_set_gate(4, 0, 0xFFFFF, 0xFF, 0x0D); // ucode
  gdt_set_gate(5, 0, 0xFFFFF, 0xF3, 0x0D); // udata
  gdt_set_gate(6, 0, 0x00000, 0xF7, 0x0D); // ustack

  gdt_set_gate(7, (u_int32) &default_tss, 0x67, 0xE9, 0x00); // TSS

  memcpy((u_int8*) gp.base, (u_int8*) gdt, gp.limit);

  __asm__ __volatile__ ("lgdtl (gdt)");

  __asm__ __volatile__ ("movw $0x10, %ax \n    \
                         movw %ax, %ds   \n    \
                         movw %ax, %es   \n    \
                         movw %ax, %fs   \n    \
                         movw %ax, %gs   \n    \
                         ljmp $0x08, $next   \n \
                         next:       \n");
}
