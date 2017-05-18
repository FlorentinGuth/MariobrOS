#ifndef LIB_PRINTER_H
#define LIB_PRINTER_H

#include "lib.h"

#define SCREEN_WIDTH  80
#define SCREEN_HEIGHT 25
#define TAB_WIDTH      4


#define ROW(pos)       ((pos) / SCREEN_WIDTH)
#define COL(pos)       ((pos) % SCREEN_WIDTH)
#define POS(row, col)  ((row) * SCREEN_WIDTH + (col))

/**
 *  @name put_char - Prints a character in the framebuffer.
 *
 *  @param i  - The location in the framebuffer
 *  @param c  - The ASCII character
 *  @param fg - The foreground color
 *  @param bg - The background color
 */
void put_char(pos_t i, char c, color_t fg, color_t bg);

/**
 *  @name pad - Replace the text with spaces (black background).
 *
 *  @param cursor_pos - The start of the space to pad (included)
 *  @param to_pad     - The end of the space to pad (excluded)
 */
void pad(pos_t cursor_pos, pos_t to_pad);

/**
 *  @name clear - Empties the framebuffer
 */
void clear();

/* /\** */
/*  *  @name scroll - Move all the text one line up, and erases the last line. */
/*  *\/ */
/* void scroll(); */

/**
 *  @name print_char - Prints a single character at the end of the framebuffer.
 *
 *  @param c - The character to print.
 */
void print_char(char c);

/**
 *  @name print_string - Prints a string at the end of the framebuffer.
 *
 *  @param string - The null-terminated string to print.
 */
void print_string(const char *string);

/**
 *  @name print_int - Prints a decimal integer at the end of the framebuffer.
 *
 *  @param n - The integer to print.
 */
void print_int(int n);

/**
 *  @name print_hex - Prints a hex integer at the end of the framebuffer.
 *
 *  @param n - The integer to print.
 */
void print_hex(int n);

/**
 *  @name printf - Prints a formatted string:
 *  - %d: a signed integer
 *  - %u: an unsigned integer
 *  - %x: an unsigned integer, in hex
 *  - %h: same as %x, but without the "0x"
 *  - %c: a char
 *  - %s: a string
 *  - %%: a real '%'
 *  - %f: changes the foreground color
 *  - %b: changes the background color
 *
 *  @param s   - Formatted string
 *  @param ... - Arguments of the formatted string
 */
void printf(char s[], ...);

/**
 * @name print_box - Prints a double-framed box to the framebuffer
 * @param upper_left -
 * @param lower_right -
 * @return void
 */
void print_box(pos_t upper_left, pos_t lower_right);

#endif
