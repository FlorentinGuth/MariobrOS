#include "kmain.h"
#include "paging.h"
#include "malloc.h"

/** kmain.c
 *  Contains the kernel main function.
 */


int kmain() /*struct multiboot mboot)*/
{
  gdt_install();
  idt_install();
  isrs_install();
  irq_install();
  __asm__ __volatile__ ("sti");

  timer_install();
  keyboard_install();

  for(unsigned int c = 128; c<256; c++) {
    writef("%d%x=%c\t",c,c,c);
  }

  write_string("Done\n");
  kloug(100, "%c%c%c%c : %c %d %x éàù", 'T','e','s','t','a', 42, 42);


  paging_install();

  clear(); /* Empties the framebuffer */


  write_string("Installing malloc...\n");
  malloc_install();
  write_string("Malloc installed!\n");

  void *a = mem_alloc(0x700000);
  write_hex((u_int32)a);
  void *b = mem_alloc(0x700000);
  write_hex((u_int32)b);
  mem_free(a);
  void *c = mem_alloc(0x700000);
  write_hex((u_int32)c);
  void *d = mem_alloc(1);
  write_hex((u_int32)d);
  mem_free(c);
  mem_free(d);
  void *e = mem_alloc(0x700000);
  write_hex((u_int32)e);

  for(;;)
    __asm__ __volatile__("hlt"); // idle state, still reacts to interrupts
  return 0xCAFEBABE;
}
