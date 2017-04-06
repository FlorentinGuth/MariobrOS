#include "kmain.h"
#include "paging.h"

/** kmain.c
 *  Contains the kernel main function.
 */


int kmain()
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

  for(unsigned char c = 0; c<255; c++) {
    write_char(c); write(", ");
  }
  write_char('\n');

  write("Victory!\n");


  /* int time = 0; */
  /* int seconds; int minutes; int hours; */
  /* while(TRUE) { */
  /*   time++; */
  /*   seconds = time%60; minutes = (time%3600 - seconds)/60; */
  /*   hours = (time%(3600*24) - minutes*60 - seconds)/3600; */
  /*   write_int(hours); write_char(':'); write_int(minutes); */
  /*   write_char(':'); write_int(seconds); write_char('\n'); */
  /*   timer_wait(1000); */
  /* } */


  for(;;)
    __asm__ __volatile__("hlt"); // idle state, still reacts to interrupts
  return 0xCAFEBABE;
}
