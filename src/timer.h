#ifndef TIMER_H
#define TIMER_H

#include "keyboard.h"

void timer_install();

/**
 *  @name timer_wait - Stops the system for a given amount of time.
 *
 *  @param amount    - The amount of time to wait, in milliseconds
 */
void timer_wait(unsigned int amount);

#endif
