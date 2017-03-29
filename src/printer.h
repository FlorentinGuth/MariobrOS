/** printer.h:
 *  Provides function to interact with the VGA framebuffer.
 *  See http://wiki.osdev.org/VGA_Hardware and http://www.osdever.net/FreeVGA/home.htm for doc.
 *  TODO: Maybe move put_char and move_cursor somewhere else, should not be manipulated outside of write.
 *  TODO: Rename to something framebuffer-explicitly-related?
 */

#ifndef PRINTER_H
#define PRINTER_H


// The framebuffer location in memory
#define FRAMEBUFFER_LOCATION 0x000B8000

// Some characteristic
#define SCREEN_WIDTH  80
#define SCREEN_HEIGHT 25
#define TAB_WIDTH      4

// The I/O ports
#define ADDRESS_REG 0x3D4
#define DATA_REG    0x3D5

// The address of the registers
#define CURSOR_HIGH_BYTE  0x0E
#define CURSOR_LOW_BYTE   0x0F


// The type of a position on the screen (whose size is 80*25)
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


/** write:
 *  Write a string at the end of the framebuffer.
 *
 *  @param string The null-terminated string to write.
 */
void write(char *string);

#endif
