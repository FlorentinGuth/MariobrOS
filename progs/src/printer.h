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
 *  @name put_char - Writes a character in the framebuffer.
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

/**
 *  @name scroll - Move all the text one line up, and erases the last line.
 */
void scroll();

/**
 * @name write_box - Writes a double-framed box to the framebuffer
 * @param upper_left -
 * @param lower_right -
 * @return void
 */
void write_box(pos_t upper_left, pos_t lower_right);

#endif
