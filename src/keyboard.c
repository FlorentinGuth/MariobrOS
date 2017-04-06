#include "keyboard.h"

unsigned char kbdus[256] =
  {
    0,  19, /* Escape */
    '&', 130, '"', '\'', '(', '-', 138, '_', /* 9 */
    135, 133, ')', '=', '\b', /* Backspace */
    '\t', /* Tab */
    'a', 'z', 'e', 'r', /* 19 */
    't', 'y', 'u', 'i', 'o', 'p', '^', '$', '\n',/* Enter key */
    0, /* 29   - Control */
    'q', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', /* 39 */
    151, 253, /* Square */
    0,/* Left shift */
    '*', 'w', 'x', 'c', 'v', 'b', 'n', /* 49 */
    ',', ';', ':', '!',   0, /* 54 - Right shift */
    '*',// * of the numeric pad
    0,/* 56 - Alt */
    ' ',/* Space bar */
    0,/* 58 - Caps lock */
    'F',/* 59 - F1 key ... > */
    'F',   'F',   'F',   'F',   'F',   'F',   'F',   'F',
    'F',/* < ... F10 */
    0,/* 69 - Num lock*/
    0,/* 70 - Scroll Lock */
    0,/* 71 - Home key */
    0,/* 72 - Up Arrow */
    24,/* Page Up */
    '-',
    0,/* 75 - Left Arrow */
    234,
    0,/* 77 - Right Arrow */
    '+',
    0,/* 79 - End key*/
    0,/* 80 - Down Arrow */
    25,/* Page Down */
    'I',/* Insert Key */
    '\177',/* Delete Key */
    234, 234, '<',
    'F', /* F11 Key */
    'F', /* F12 Key */
    234,234,5, /* Windows key */
    234,219, /* Menu */
    234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,

    /* With shift */
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
    '9', '0', 0, '+', '\b', /* Backspace */
    '\t', /* Tab */
    'A', 'Z', 'E', 'R', /* 19 */
    'T', 'Y', 'U', 'I', 'O', 'P', 137, 156, '\n',/* Enter key */
    0, /* 29   - Control */
    'Q', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M', /* 39 */
    '%', 234,   0, /* Left shift */
    230, 'W', 'X', 'C', 'V', 'B', 'N', /* 49 */
    '?', '.', '/', '!',   0, /* 54 - Right shift */
    234,
    0,/* 56 - Alt */
    ' ',/* Space bar */
    0,/* 58 - Caps lock */
    'F',/* 59 - F1 key ... > */
    'F',   'F',   'F',   'F',   'F',   'F',   'F',   'F',
    'H',/* < ... F10 */
    0,/* 69 - Num lock*/
    0,/* 70 - Scroll Lock */
    0,/* 71 - Home key */
    0,/* 72 - Up Arrow */
    24,/* Page Up */
    '-',
    0,/* 75 - Left Arrow */
    '?',
    0,/* 77 - Right Arrow */
    '+',
    0,/* 79 - End key*/
    0,/* 80 - Down Arrow */
    25,/* Page Down */
    'I',/* Insert Key */
    '\177',/* Delete Key */
    '?',   '?',   '>',
    'F', /* F11 Key */
    'F', /* F12 Key */
    234,234,5, /* Windows key */
    234,219, /* Menu */
    234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,
  };

void keyboard_set_lights(unsigned char signal)
{
  while(TRUE) {
    if(!(inb(0x64) & 2)) // Wait for the keyboard controller to be idle
      break;
  }
  outb(0x60, 0xed);
  outb(0x60, signal);
}

bool k_caps_lock = FALSE;
bool k_num_lock = FALSE;
bool k_scroll_lock = FALSE;
bool k_shift_count = 0;
bool k_shift = FALSE;
bool k_meta = 0;
bool k_ctrl = 0;
unsigned char k_lights = 0x00;
/* Bit:     | 7       3 |  2  |  1  |  0  |
   Content: | 0 0 0 0 0 | Cap | Num | Scr | */

/* Handles the keyboard interrupt */
#pragma GCC diagnostic ignored "-Wunused-parameter"
void keyboard_handler(struct regs *r)
{
  int scancode;

  /* Read from the keyboard's data buffer */
  scancode = inb(0x60);
  /* If the top bit of the byte we read from the keyboard is
   * set, that means that a key has just been released */
  if (scancode & 0x80)
    {
      switch(scancode^0x80) {

      case 29: { // Ctrl
        k_ctrl--;
        break;
      }
      case 42: { // Left Shift
        k_shift_count--;
        k_shift = ((k_lights&0x04)>>2) ^ k_shift_count;
        // k_shift = k_caps_lock ^ k_shift_count
        break;
      }
      case 54: { // Right Shift
        k_shift_count--;
        k_shift = ((k_lights&0x04)>>2) ^ k_shift_count;
        // k_shift = k_caps_lock ^ k_shift_count
        break;
      }
      case 56: { // Alt
        k_meta--;
        break;
      }

      }
    }
  else
    {
      switch (scancode) {

      case 29: { // Ctrl
        k_ctrl++;
        break;
      }
      case 42: { // Left Shift
        k_shift_count++;
        k_shift = ((k_lights&0x04) ^ 0x04)>>2;
        break;
      }
      case 54: { // Right Shift
        k_shift_count++;
        k_shift = ((k_lights&0x04) ^ 0x04)>>2;
        break;
      }
      case 56: { // Alt
        k_meta++;;
        break;
      }
      case 58: { // Caps lock
        k_lights = k_lights ^ 0x04;
        k_shift =!k_shift;
        keyboard_set_lights(k_lights);
        break;
      }
      case 69: { // Num lock
        k_lights = k_lights ^ 0x02;
        keyboard_set_lights(k_lights);
        break;
      }
      case 70: { // Scroll lock
        k_lights = k_lights ^ 0x01;
        keyboard_set_lights(k_lights);
        break;
      }
        /* Next cases are only of use in the framebuffer */
      case 71: { // Home
        pos_t pos = get_cursor_pos();
        set_cursor_pos(SCREEN_WIDTH * (pos / SCREEN_WIDTH));
        break;
      }
      case 72: { // Up
        pos_t pos = get_cursor_pos();
        set_cursor_pos((pos-SCREEN_WIDTH) % (SCREEN_WIDTH*SCREEN_HEIGHT));
        break;
      }
      case 75: { // Left
        pos_t pos = get_cursor_pos();
        set_cursor_pos((pos-1) % (SCREEN_WIDTH*SCREEN_HEIGHT));
        break;
      }
      case 77: { // Right
        pos_t pos = get_cursor_pos();
        set_cursor_pos((pos+1) % (SCREEN_WIDTH*SCREEN_HEIGHT));
        break;
      }
      case 79: { // End
        pos_t pos = get_cursor_pos();
        set_cursor_pos(SCREEN_WIDTH *((pos / SCREEN_WIDTH) + 1) - 1);
        break;
      }
      case 80: { // Down
        pos_t pos = get_cursor_pos();
        set_cursor_pos((pos+SCREEN_WIDTH) % (SCREEN_WIDTH*SCREEN_HEIGHT));
        break;
      }
      default: {
        char c = kbdus[(scancode+(k_shift*128))];
        write_char(c);
      }
      }

      /* Just to show you how this works, we simply translate
       *  the keyboard scancode into an ASCII value, and then
       *  display it to the screen. You can get creative and
       *  use some flags to see if a shift is pressed and use a
       *  different layout, or you can add another 128 entries
       *  to the above layout to correspond to 'shift' being
       *  held. If shift is held using the larger lookup table,
       *  you would add 128 to the scancode when you look for it */
    }

}
#pragma GCC diagnostic pop


void keyboard_install()
{
  irq_install_handler(1, keyboard_handler);
}
