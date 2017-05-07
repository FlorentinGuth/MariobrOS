/** idle.c:
 *  Code for the idle process (does nothing).
 */

int main()
{
  for (;;) {
    asm volatile ("hlt");
  }
  return 0;  /* Dead code */
}
