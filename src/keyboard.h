#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "irq.h"

/**
 *  @name keyboard_install - Enables keyboard support
 *  @param shell - Whether to send the keys to the sell or write them down directly
 *  @return void
 */
void keyboard_install(bool shell);

#endif
