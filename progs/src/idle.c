#include "lib.h"


/** idle.c:
 *  Code for the idle process (does nothing).
 */

int main()
{
  for (;;) {
    hlt(TRUE);
  }
  return 0;  /* Dead code */
}
