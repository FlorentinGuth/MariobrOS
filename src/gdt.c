#include "gdt.h"
#include "io.h"
#include "logging.h"

/* Source : http://www.osdever.net/bkerndev/Docs/gdt.htm */

#define GDT_SIZE 4

/* Our GDT, with 3 entries, and finally our special GDT pointer */
struct gdt_entry gdt[GDT_SIZE];
struct gdt_ptr gp;
struct tss tss;

/* Setup a descriptor in the Global Descriptor Table */
void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran)
{
  /* Setup the descriptor base address */
  gdt[num].base_low = (base & 0xFFFF);
  gdt[num].base_middle = (base >> 16) & 0xFF;
  gdt[num].base_high = (base >> 24) & 0xFF;

  /* Setup the descriptor limits */
  gdt[num].limit_low = (limit & 0xFFFF);
  gdt[num].granularity = ((limit >> 16) & 0x0F);

  /* Finally, set up the granularity and access flags */
  gdt[num].granularity |= (gran & 0xF0);
  gdt[num].access = access;
}


/* Should be called by main. This will setup the special GDT
 * pointer, set up the first 3 entries in our GDT, and then
 * finally call gdt_flush() in our assembler file in order
 * to tell the processor where the new GDT is and update the
 * new segment registers */
void gdt_install()
{
  /* Setup the GDT pointer and limit */
  gp.limit = (sizeof(struct gdt_entry) * GDT_SIZE) - 1;
  gp.base = (unsigned int) &gdt;

  tss.debug_flag = 0x00;
  tss.io_map = 0x00;
  tss.esp0 = 0x1FFF0;
  tss.ss0 = 0x18;

  /* Our NULL descriptor */
  gdt_set_gate(0, 0, 0, 0, 0);

  /* The second entry is our Code Segment. The base address
   * is 0, the limit is 4GBytes, it uses 4KByte granularity,
   * uses 32-bit opcodes, and is a Code Segment descriptor.
   * Please check the table above in the tutorial in order
   * to see exactly what each value means */
  gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

  /* The third entry is our Data Segment. It's EXACTLY the
   * same as our code segment, but the descriptor type in
   * this entry's access byte says it's a Data Segment */
  gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

  gdt_set_gate(3, (u_int32) &tss, 0x67, 0xE9, 0x00);

  /* Flush out the old GDT and install the new changes! */
  gdt_flush();

  kloug(100, "GTD installed\n");
}

void init_pic()
{
    /* Initialization of ICW1 */
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    /* Initialization of ICW2 */
    outb(0x21, 0x20);	/* start vector of master PIC = 32 */
    outb(0xA1, 0x28);	/* start vector of slave PIC = 40 */
    //outb(0xA1, 0x70);	/* start vector of slave PIC = 112 */

    /* Initialization of ICW3 */
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    /* Initialization of ICW4 */
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    /* mask interrupts */
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    kloug(100, "PIC installed\n");
}
