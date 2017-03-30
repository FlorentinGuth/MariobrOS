#include "printer.h"
#include "logging.h"
#include "segmentation.h"

/** kmain.c
 *  Contains the kernel main function.
 */


int kmain()
{
  char string[] = "Hello World!";
  char currI[] = "\tnumber   \n";
  int units; int tens;
  for(int i=0; i < 32; i++) {
    write(string);
    units = i%10; tens = ((i%100)-units)/10;
    currI[8] = tens + '0';
    currI[9] = units + '0';
    write(currI);
  }

  gdt_e GDT[3];
  segmentize(GDT);

  
  

  log("Grâce à ce vieux hack immonde, j'ai réussi à écrire autant de texte que je voulais dans le buffer ; l'astuce va vous surprendre !\n\n",   Info);
  log("Je ne suis pas sûr que ce soit une très bonne solution...\n", Debug);
  log("Mais elle marche, ou au moins sur un texte qui est raisonnablement long pour une erreur, mais pas non plus terriblement excessif.\nSi on s'amuse à écrire des romans dans les messages d'erreur, ça pourrait effectivement finir par ne plus marcher, mais de toute façon ce hack ne me semble pas être une solution très viable.", Error);

  return 0xCAFEBABE;
}
