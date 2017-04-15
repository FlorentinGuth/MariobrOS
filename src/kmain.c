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

  malloc_install();

  void *a = mem_alloc(4);
  writef("a:%x\n",a);
  void* b = mem_alloc(4);
  writef("b:%x\n",b);
  void *c = mem_alloc(4);
  writef("c:%x\n",c);
  mem_free(b);
  void* d = mem_alloc(4);
  writef("d:%x\n",d);
  void* e = mem_alloc(4);
  writef("e:%x\n",e);
  
  for(;;)
    __asm__ __volatile__("hlt"); // idle state, still reacts to interrupts
  return 0xCAFEBABE;
}
