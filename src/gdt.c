#include "gdt.h"
#include "io.h"
#include "logging.h"
#include "error.h"
#include "memory.h"

/* Source : http://www.osdever.net/bkerndev/Docs/gdt.htm */

#define GDT_SIZE 8
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
    gdt[current_entry].executable  = TRUE;   /* Code segment */
    gdt[current_entry].dir_conform = FALSE;  /* Code executable by anyone */
  } else {
    /* Data segment */
    gdt[current_entry].executable  = FALSE; /* Data segment */
    gdt[current_entry].dir_conform = FALSE; /* Segment growing upward */
  }

  /* Flags */
  gdt[current_entry].access      = FALSE;          /* Set to 0 by default */
  gdt[current_entry].always_one  = TRUE;
  gdt[current_entry].present     = base != limit;  /* Detects the null sector */
  gdt[current_entry].available   = 1;
  gdt[current_entry].always_zero = 0;
  gdt[current_entry].size        = TRUE;           /* 32 bits */
  gdt[current_entry].granularity = TRUE;           /* Page granularity */

  /* Privilege stuff */
  gdt[current_entry].privilege   = ring;
  gdt[current_entry].read_write  = TRUE;

  *address = current_entry * sizeof(gdt_entry_t);
  current_entry++;
}

void write_tss(gdt_entry_t* g)
{
   // Firstly, let's compute the base and limit of our entry into the GDT.
   u_int32 base = (u_int32) &TSS_ENTRY;
   u_int32 limit = sizeof(TSS_ENTRY);

   // Now, add our TSS descriptor's address to the GDT.
   g->limit_low = limit & 0xFFFF;
   g->base_low = base & 0xFFFF;
   g->base_middle = (base >> 16) & 0xFF;
   g->access = 1; //This indicates it's a TSS and not a LDT. This is a changed meaning
   g->read_write = 0; //This indicates if the TSS is busy or not. 0 for not busy
   g->dir_conform = 0; //always 0 for TSS
   g->executable = 1; //For TSS this is 1 for 32bit usage, or 0 for 16bit.
   g->always_one = 0; //indicate it is a TSS
   g->privilege = 3; //same meaning
   g->present = 1; //same meaning
   g->limit_high = (limit >> 16) & 0xF;
   g->available = 0;
   g->always_zero = 0; //same thing
   g->size = 0; //should leave zero according to manuals. No effect
   g->granularity = 0; //so that our computed GDT limit is in bytes, not pages
   g->base_high = (base >> 24) & 0xFF; //isolate top byte.

   // Ensure the TSS is initially zero'd.
   mem_set(&TSS_ENTRY, 0, sizeof(TSS_ENTRY));

   TSS_ENTRY.ss0  = KERNEL_STACK_SEGMENT;  // Set the kernel stack segment.
   TSS_ENTRY.esp0 = START_OF_KERNEL_STACK; // Set the kernel stack pointer.
   //note that CS is loaded from the IDT entry and should be the regular kernel code segment
   current_entry++;
}

/* This will update the ESP0 stack used when an interrupt occurs */
void set_kernel_stack(u_int32 stack)
{
   TSS_ENTRY.esp0 = stack;
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

  /* Our NULL descriptor */
  add_segment(&NULL_SEGMENT, 0, 0, FALSE, 0);

  /* The second entry is our Code Segment. The base address
   * is 0, the limit is 4GBytes, it uses 4KByte granularity,
   * uses 32-bit opcodes, and is a Code Segment descriptor.
   * Please check the table above in the tutorial in order
   * to see exactly what each value means */
  add_segment(&KERNEL_CODE_SEGMENT,  0, 0xFFFFF, TRUE, 0);

  /* The third entry is our Data Segment. It's EXACTLY the
   * same as our code segment, but the descriptor type in
   * this entry's access byte says it's a Data Segment */
  add_segment(&KERNEL_DATA_SEGMENT,  0, 0xFFFFF, FALSE, 0);

  add_segment(&KERNEL_STACK_SEGMENT, 0, 0xFFFFF, FALSE, 0);
  /* gdt[KERNEL_STACK_SEGMENT].dir_conform = 1; */

  /* Now the user segments - just the same as the kernel ones */
  add_segment(&USER_CODE_SEGMENT,  0, 0xFFFFF, TRUE,  3);
  add_segment(&USER_DATA_SEGMENT,  0, 0xFFFFF, FALSE, 3);
  add_segment(&USER_STACK_SEGMENT, 0, 0xFFFFF, FALSE, 3);
  /* gdt[USER_STACK_SEGMENT].dir_conform = 1; */

  write_tss(&gdt[current_entry]);
  /* Flush out the old GDT and install the new changes! */
  gdt_flush();

  tss_flush();

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
