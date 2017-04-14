#include "kmain.h"
#include "paging.h"

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
  paging_install();

  clear(); // Empties the framebuffer


  for(unsigned int c = 128; c<256; c++) {
    writef("%d%x=%c\t",c,c,c);
  }

  write_string("Done\n");
  kloug(100, "%c%c%c%c : %c %d %x éàù", 'T','e','s','t','a', 42, 42);

  for(;;)
    __asm__ __volatile__("hlt"); // idle state, still reacts to interrupts
  return 0xCAFEBABE;
}
