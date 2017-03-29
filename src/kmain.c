#include "printer.h"

/** kmain.c
 *  Contains the kernel main function.
 */


int kmain()
{
  char string[] = "Hello World!\nNew line!\tAnd tabulation!\ta\taa\taaa\taaaa\taaaaaaaaaaaaaaaaaaaaaaaaaaaa";
  write(string);

  return 0xCAFEBABE;
}
