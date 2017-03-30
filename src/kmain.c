#include "printer.h"
#include "logging.h"
#include "gdt.h"

/** kmain.c
 *  Contains the kernel main function.
 */


int kmain()
{
  gdt_install();
  
  char string[] = "J'ai fini par tout pomper d'un site, apres avoir passe au moins 4h a essayer de trouver ou etait le bug dans mon code... Et je ne sais toujours pas ou il etait.\n\nCe projet me fatigue !";
  write(string);

  return 0xCAFEBABE;
}
