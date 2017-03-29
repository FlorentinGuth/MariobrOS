#include "printer.h"

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

  write("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");

  return 0xCAFEBABE;
}
