#ifndef PRINTER_H
#define PRINTER_H

/** printer.h:
 *  Provides function to interact with the VGA framebuffer.
 *  See http://wiki.osdev.org/VGA_Hardware and http://www.osdever.net/FreeVGA/home.htm for doc.
 *  TODO: Maybe move put_char and move_cursor somewhere else, should not be manipulated outside of write.
 *  TODO: Rename to something framebuffer-explicitly-related?
 */

#include "types.h"
#include "io.h"
#include "string.h"

/* Some characteristic */
#define SCREEN_WIDTH  80
#define SCREEN_HEIGHT 25
#define TAB_WIDTH      4

char * const framebuffer;

#define ROW(pos)       ((pos) / SCREEN_WIDTH)
#define COL(pos)       ((pos) % SCREEN_WIDTH)
#define POS(row, col)  ((row) * SCREEN_WIDTH + (col))


/* The I/O ports */
#define ADDRESS_REG (port_t)0x3D4
#define DATA_REG    (port_t)0x3D5

/* The address of the registers */
#define CURSOR_HIGH_BYTE  0x0E
#define CURSOR_LOW_BYTE   0x0F


/**
 *  @name pos_t - The type of a position on the screen (whose size is 80*25)
 *  Note that a pos_t never accounts for the fact that each location takes up two bytes.
 */
typedef u_int16 pos_t;

enum Color {
  Black        =  0,
  Blue         =  1,
  Green        =  2,
  Cyan         =  3,
  Red          =  4,
  Magenta      =  5,
  Brown        =  6,
  LightGrey    =  7,
  DarkGrey     =  8,
  LightBlue    =  9,
  LightGreen   = 10,
  LightCyan    = 11,
  LightRed     = 12,
  LightMagenta = 13,
  LightBrown   = 14,
  White        = 15,
};
typedef enum Color color_t;


/**
 *  @name put_char - Writes a character in the framebuffer.
 *
 *  @param i  - The location in the framebuffer
 *  @param c  - The ASCII character
 *  @param fg - The foreground color
 *  @param bg - The background color
 */
void put_char(pos_t i, char c, color_t fg, color_t bg);

/**
 *  @name get_cursor_pos -
 *  Communicate with the framebuffer to get the cursor position.
 *
 *  @return The position of the cursor
 */
pos_t get_cursor_pos();

/**
 *  @name set_cursor_pos - Moves the cursor of the framebuffer.
 *
 *  @param pos The new position of the cursor
 */
void set_cursor_pos(pos_t pos);


/**
 *  @name scroll - Move all the text one line up, and erases the last line.
 */
void scroll();

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

/**
 *  @name write_char - Writes a single character at the end of the framebuffer.
 *
 *  @param c - The character to write.
 */
void write_char(char c);

/**
 *  @name write_string - Writes a string at the end of the framebuffer.
 *
 *  @param string - The null-terminated string to write.
 */
void write_string(const char *string);

/**
 *  @name write_int - Writes a decimal integer at the end of the framebuffer.
 *
 *  @param n - The integer to write.
 */
void write_int(int n);

/**
 *  @name write_hex - Writes a hex integer at the end of the framebuffer.
 *
 *  @param n - The integer to write.
 */
void write_hex(int n);

/**
 *  @name writef - Writes a formatted string:
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
void writef(char s[], ...);

/**
 * @name write_box - Writes a double-framed box to the framebuffer
 * @param upper_left -
 * @param lower_right -
 * @return void
 */
void write_box(pos_t upper_left, pos_t lower_right);

#endif
