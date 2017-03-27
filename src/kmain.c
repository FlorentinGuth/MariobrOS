#include "printer.h"

int kmain()
{
  char string[] = "Hello World!";
  write(string, sizeof(string)/sizeof(char) - 1); // Account for /0 trailing char
  return 0xCAFEBABE;
}
