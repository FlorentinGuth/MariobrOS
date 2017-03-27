#ifndef PRINTER_H
#define PRINTER_H


// TODO: maybe move put_char and move_cursor somewhere else, should not be manipulated outside of write
// TODO: rename to something framebuffer-explicitly-related?

// The framebuffer location in memory
#define FRAMEBUFFER_LOCATION 0x000B8000

// The I/O ports
#define COMMAND_PORT 0x3D4
#define DATA_PORT    0x3D5

// The I/O ports commands
#define HIGH_BYTE_COMMAND 14
#define LOW_BYTE_COMMAND  15


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
void put_char(unsigned int i, char c, color_t fg, color_t bg);

/** move_cursor:
 *  Moves the cursor of the framebuffer.
 *
 *  @param pos The new position of the cursor
 */
void move_cursor(unsigned short pos);

/** write:
 *  Write a string at the end of the framebuffer.
 *  TODO: deal with cursor and scrolling.
 *
 *  @param buf The string to write
 *  @param len The number of bytes to write
 */
void write(char *buf, unsigned int len);

#endif
