#ifndef ERROR_H
#define ERROR_H


#include "printer.h"

/**
 *  @name throw - Prints the string to the screen, along with the location of
 *  the error, and loop forever.
 */
#define throw(string) \
  write_string("\nFile "); \
  write_string(__FILE__); \
  write_string(", line "); \
  write_int(__LINE__); \
  write_string(": ERROR "); \
  write_string(string); \
  \
  for(;;) {\
    __asm__ __volatile__("hlt");} // idle state, still reacts to interrupts

#endif
