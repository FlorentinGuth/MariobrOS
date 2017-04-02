#include "printer.h"
#include "logging.h"
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"

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
  
  write("On teste l'erreur 0x10.\n");
  
  __asm__("int $0x10");
  
  write("Froude !");
  
  return 0xCAFEBABE;
}
