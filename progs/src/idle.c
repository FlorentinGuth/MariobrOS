#include "lib.h"


/** idle.c:
 *  Code for the idle process (does nothing).
 */

int main()
{
  for (;;) {
    syscall_hlt();
  }
  return 0;  /* Dead code */
}
