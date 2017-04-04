#include "printer.h"
#include "logging.h"
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "timer.h"
#include "keyboard.h"

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

  /* Essaie d'ajouter les minutes et les heures... BOCHS refuse chez moi. */

  unsigned int time = 0;

  while(TRUE) {
    write_int(time);
    timer_wait(500);
    write_char(','); write_char(' ');
    timer_wait(500);
    time++;
  }
  return 0xCAFEBABE;
}
