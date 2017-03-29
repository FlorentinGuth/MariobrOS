#include "printer.h"

int kmain()
{
  char string[] = "Hello World!";
  write(string, sizeof(string)/sizeof(char));
  return 0xCAFEBABE;
}
