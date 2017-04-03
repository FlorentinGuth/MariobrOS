#include "printer.h"
#include "isr.h"
#include "irq.h"
#include "io.h"
#include "logging.h"
#include "string.h"

unsigned char kbdus[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
  '9', '0', '-', '=', '\b', /* Backspace */
  '\t', /* Tab */
  'q', 'w', 'e', 'r', /* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    'C', /* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
 '\'', '!',   'M', /* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', /* 49 */
  'm', ',', '.', '/',   'M', /* Right shift */
  '*',
    'A',/* Alt */
  ' ',	/* Space bar */
    'C',/* Caps lock */
    'F', /* 59 - F1 key ... > */
    'F',   'F',   'F',   'F',   'F',   'F',   'F',   'F',
    'H',/* < ... F10 */
    'N',/* 69 - Num lock*/
    'S',/* Scroll Lock */
    'K',/* Home key */
    'U',/* Up Arrow */
    '?',/* Page Up */
  '-',
    'L',/* Left Arrow */
    '?',
    'R',/* Right Arrow */
  '+',
    'E',/* 79 - End key*/
    'B',/* Down Arrow */
    '?',/* Page Down */
    'I',/* Insert Key */
    'D',/* Delete Key */
    '?',   '?',   '?',
    'F', /* F11 Key */
    'F', /* F12 Key */
    0,	/* All other keys are undefined */
};

/* Handles the keyboard interrupt */
#pragma GCC diagnostic ignored "-Wunused-parameter"
void keyboard_handler(struct regs *r)
{
    int scancode;

    /* Read from the keyboard's data buffer */
    scancode = inb(0x60);
    /* If the top bit of the byte we read from the keyboard is
    *  set, that means that a key has just been released */
    if (scancode & 0x80)
    {
        /* You can use this one to see if the user released the
        *  shift, alt, or control keys... */
    }
    else
    {
        /* Here, a key was just pressed. Please note that if you
        *  hold a key down, you will get repeated key press
        *  interrupts. */

        /* Just to show you how this works, we simply translate
        *  the keyboard scancode into an ASCII value, and then
        *  display it to the screen. You can get creative and
        *  use some flags to see if a shift is pressed and use a
        *  different layout, or you can add another 128 entries
        *  to the above layout to correspond to 'shift' being
        *  held. If shift is held using the larger lookup table,
        *  you would add 128 to the scancode when you look for it */
      char c = kbdus[scancode];
      write_char(c);
    }
    
}
#pragma GCC diagnostic pop


void keyboard_install()
{
  irq_install_handler(1, keyboard_handler);
}
