#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "irq.h"

/**
 *  @name keyboard_install - Enables keyboard support
 *  @param shell - Whether to send the keys to the sell or write them down directly
 */
void keyboard_install(bool shell);

/**
 *  @name keyboard_get - Gives the first non read scancode (if shell is on)
 */
u_int8 keyboard_get();

#endif
