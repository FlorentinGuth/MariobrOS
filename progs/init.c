/** init.c:
 *  Code for the init process (calls wait for zombie orphans).
 */

#include "lib.h"

int main()
{
  u_int32 pid, return_value;
  for (;;) {
    syscall_wait(&pid, &return_value);
  }
  return 0;  /* Dead code */
}
