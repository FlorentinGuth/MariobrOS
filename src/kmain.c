#include "printer.h"
#include "logging.h"
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "timer.h"

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

  /* Essaie d'ajouter les minutes et les heures... BOCHS refuse chez moi. */

  char seconds;
  unsigned int time = 0;
  char buf[3];
  buf[2] = '\0';

  while(TRUE) {
    seconds = time%60;
    write("Time is now: ");
    buf[0] = seconds/10 + '0'; buf[1] = seconds%10 + '0';
    write(buf); write("\n");
    timer_wait(1000);
    time++;
  }

  return 0xCAFEBABE;
}
