#include "printer.h"
#include "logging.h"
#include "gdt.h"
#include "idt.h"
#include "isr.h"

/** kmain.c
 *  Contains the kernel main function.
 */


int kmain()
{
  gdt_install();
  idt_install();
  isrs_install();

  write("Dans quelques instants, je vais diviser par 0...\n");
  
  log("L'erreur sera-t-elle rattrapee ? (le suspense est a son comble)", Info);
  #pragma GCC diagnostic ignored "-Wunused-variable"
  #pragma GCC diagnostic ignored "-Wdiv-by-zero"
  int x = 3/0;
  
  // Est-ce que j'écris l'erreur dans le log ou à l'écran ?
  // Pour l'instant j'ai coupé la poire en deux...
  
  log("Apres", Info);
  
  write("Froude !");
  
  return 0xCAFEBABE;
}
