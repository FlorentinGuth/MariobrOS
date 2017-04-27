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

/* The I/O ports */
#define ADDRESS_REG (port_t)0x3D4
#define DATA_REG    (port_t)0x3D5

/* The address of the registers */
#define CURSOR_HIGH_BYTE  0x0E
#define CURSOR_LOW_BYTE   0x0F


/** pos_t:
 *  The type of a position on the screen (whose size is 80*25)
 *  Note that a pos_t never accounts for the fact that each location takes up two bytes.
 */
typedef unsigned short pos_t;

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


/** put_char:
 *  Writes a character in the framebuffer.
 *
 *  @param i  The location in the framebuffer
 *  @param c  The ASCII character
 *  @param fg The foreground color
 *  @param bg The background color
 */
void put_char(pos_t i, char c, color_t fg, color_t bg);

/** get_cursor_pos:
 *  Communicate with the framebuffer to get the cursor position.
 *
 *  @return The position of the cursor
 */
pos_t get_cursor_pos();

/** set_cursor_pos:
 *  Moves the cursor of the framebuffer.
 *
 *  @param pos The new position of the cursor
 */
void set_cursor_pos(pos_t pos);


/** scroll:
 *  Move all the text one line up, and erases the last line.
 */
void scroll();

/** pad:
 *  Replace the text with spaces (black background).
 *
 *  @param cursor_pos the start of the space to pad (included)
 *  @param to_pad     the end of the space to pad (excluded)
 */
void pad(pos_t cursor_pos, pos_t to_pad);

/** clear:
 *  Empties the framebuffer
 */
void clear();

/** write_char:
 *  Writes a single character at the end of the framebuffer.
 *
 *  @param c The character to write.
 */
void write_char(char c);

/** write_string:
 *  Writes a string at the end of the framebuffer.
 *
 *  @param string The null-terminated string to write.
 */
void write_string(const char *string);

/** write_int:
 *  Writes an integer at the end of the framebuffer (in base 10).
 *
 *  @param n The integer to write.
 */
void write_int(int n);

/** write_hex:
 *  Writes a hex integer at the end of the framebuffer.
 *
 *  @param n The integer to write.
 */
void write_hex(int n);

/** writef:
 *  Writes a formatted string:
 *  - %d: a signed integer
 *  - %u: an unsigned integer
 *  - %x: an unsigned integer, in hex
 *  - %c: a char
 *  - %s: a string
 *  - %f: changes the foreground color
 *  - %b: changes the background color
 *  - %%: writes a '%'
 *
 *  @param s       Formatted string
 *  @param ...     Arguments of the formatted string
 */
void writef(char s[], ...);

#endif
