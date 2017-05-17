#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "irq.h"

/**
 *  @name keyboard_install - Enables keyboard support
 *  @param shell_status    - 0 for no shell (directly writes characters to the screen)
 *                         - 1 for the kernel shell (transmits the keys to the internal shell)
 *                         - 2 for the user shell (key presses are buffered)
 */
void keyboard_install(int shell_status);

/**
 *  @name keyboard_get - Gives the first non read scancode (if shell is on)
 */
u_int8 keyboard_get();

#endif
