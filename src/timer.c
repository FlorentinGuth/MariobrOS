#include "io.h"
#include "irq.h"

/* Source : http://www.osdever.net/bkerndev/Docs/pit.htm */

void timer_phase(int hz)
{
    int divisor = 1193180 / hz;   /* Calculate our divisor */
    /* Bit:     | 7 6 | 5 4 | 3   1 | 0 |
     * Content: | CNTR|  RW  | Mode |BCD|
     * Value:   | 0 0 | 1 1 | 0 1 1 | 0 |= 0x36 */
    outb(0x43, 0x36);             /* Set our command byte 0x36 */
    outb(0x40, divisor & 0xFF);   /* Set low byte of divisor */
    outb(0x40, divisor >> 8);     /* Set high byte of divisor */
}

/* This will keep track of how many ticks that the system
*  has been running for */
unsigned int timer_ticks = 0;

/* Handles the timer. In this case, it's very simple: We
*  increment the 'timer_ticks' variable every time the
*  timer fires. By default, the timer fires 18.222 times
*  per second. */

#pragma GCC diagnostic ignored "-Wunused-parameter"
void timer_handler(struct regs *r)
{
  /* Increment our 'tick count' */
  timer_ticks++;
}
#pragma GCC diagnostic pop

/* Sets up the system clock by installing the timer handler
*  into IRQ0 */
void timer_install()
{
  timer_phase(1000);
  /* Installs 'timer_handler' to IRQ0 */
  irq_install_handler(0, timer_handler);
}

void timer_wait(unsigned int amount)
{
  /* Stops the system for @param amount milliseconds*/
  unsigned int t = timer_ticks + amount;
  while(timer_ticks < t);
}
