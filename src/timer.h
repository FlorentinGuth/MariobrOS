#ifndef TIMER_H
#define TIMER_H

void timer_install();

/* timer_wait:
 * Stops the system for a given amount of time
 * @param amount : amount of time to wait, in milliseconds
 */

void timer_wait(unsigned int amount);

#endif
