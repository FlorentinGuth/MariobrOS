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

  log("Test", Debug);
  log("TEST", Error);
  log("Blàbléblù", Info);


  for(;;);
  return 0xCAFEBABE;
}
