#include "gdt.h"
#include "io.h"
#include "logging.h"
#include "error.h"

/* Source : http://www.osdever.net/bkerndev/Docs/gdt.htm */

#define GDT_SIZE 6
int current_entry = 0;

/* Our GDT, with 6 entries, and finally our special GDT pointer */
gdt_entry_t gdt[GDT_SIZE];
gdt_ptr_t gp;
tss_t tss;

/**
 * @name add_segment - Adds an entry in the GDT table
 * @param segment    - Will be set to the offset in the GDT
 * @param base       - Base address of the segment
 * @param limit      - High address of the segment
 * @param code       - TRUE for a code segment, FALSE for a data segment
 * @return void
 */
void add_segment(u_int32 *address, u_int32 base, u_int32 limit, bool code, int ring)
{
  if (current_entry == GDT_SIZE) {
    throw("Too many segments!");
  }

  /* Setup the descriptor base address */
  gdt[current_entry].base_low = (base & 0xFFFF);
  gdt[current_entry].base_middle = (base >> 16) & 0xFF;
  gdt[current_entry].base_high = (base >> 24) & 0xFF;

  /* Setup the descriptor limit */
  gdt[current_entry].limit_low = (limit & 0xFFFF);
  gdt[current_entry].limit_high = (limit >> 16) & 0xF;

  /* Code or data segment stuff */
  if (code) {
    gdt[current_entry].executable  = TRUE;  /* Code segment */
    gdt[current_entry].dir_conform = TRUE;  /* Code executable by anyone */
  } else {
    /* Data segment */
    gdt[current_entry].executable  = FALSE; /* Data segment */
    gdt[current_entry].dir_conform = FALSE; /* Segment growing upward */
  }

  /* Flags */
  gdt[current_entry].access      = FALSE;          /* Set to 0 by default */
  gdt[current_entry].always_one  = TRUE;
  gdt[current_entry].present     = base != limit;  /* Detects the null sector */
  gdt[current_entry].always_zero = 0;
  gdt[current_entry].size        = TRUE;           /* 32 bits */
  gdt[current_entry].granularity = TRUE;           /* Page granularity */

  /* Privilege stuff */
  gdt[current_entry].privilege   = ring;
  gdt[current_entry].read_write  = TRUE;

  *address = current_entry * sizeof(gdt_entry_t);
  current_entry++;
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
  gp.base = (u_int32)&gdt;

  tss.debug_flag = 0x00;
  tss.io_map = 0x00;
  tss.esp0 = 0x1FFF0;
  tss.ss0 = 0x18;

  /* Our NULL descriptor */
  add_segment(&NULL_SEGMENT, 0, 0, FALSE, 0);

  /* The second entry is our Code Segment. The base address
   * is 0, the limit is 4GBytes, it uses 4KByte granularity,
   * uses 32-bit opcodes, and is a Code Segment descriptor.
   * Please check the table above in the tutorial in order
   * to see exactly what each value means */
  add_segment(&KERNEL_CODE_SEGMENT, 0, 0xFFFFFFFF, TRUE, 0);

  /* The third entry is our Data Segment. It's EXACTLY the
   * same as our code segment, but the descriptor type in
   * this entry's access byte says it's a Data Segment */
  add_segment(&KERNEL_DATA_SEGMENT, 0, 0xFFFFFFFF, FALSE, 0);

  /* The TSS segment. Not sure if this are the right parameters */
  /* access = 0xE9 = 11101001, granularity = 0x00 = 00000000 */
  /* add_segment(&TSS_SEGMENT, (u_int32)&tss, 0x67, FALSE); */
  /* gdt[tss_seg].access      = 1; */
  /* gdt[tss_seg].read_write  = 0; */
  /* gdt[tss_seg].dir_conform = 0; */
  /* gdt[tss_seg].executable  = 1; */
  /* gdt[tss_seg].always_one  = 1; */
  /* gdt[tss_seg].privilege   = 3; */
  /* gdt[tss_seg].present     = 1; */
  /* gdt[tss_seg].always_zero = 0; */
  /* gdt[tss_seg].size        = 0; */
  /* gdt[tss_seg].granularity = 1; */

  /* Now the user segments - just the same as the kernel ones */
  add_segment(&USER_CODE_SEGMENT, 0, 0xFFFFFFFF, TRUE,  3);
  add_segment(&USER_DATA_SEGMENT, 0, 0xFFFFFFFF, FALSE, 3);

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
