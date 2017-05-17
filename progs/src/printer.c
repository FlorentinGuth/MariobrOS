#include "printer.h"

color_t foreground = White;
color_t background = Black;

#define VERTICAL_BAR       186
#define UPPER_RIGHT_CORNER 187
#define LOWER_RIGHT_CORNER 188
#define LOWER_LEFT_CORNER  200
#define UPPER_LEFT_CORNER  201
#define HORIZONTAL_BAR     205

#define PUT_CHAR(row, col, c) put_char(POS(row, col), (char)c, foreground, background)

void put_char(pos_t i, char c, color_t fg, color_t bg)
{
  /* Each character actually takes up two bytes */
  set_char(2*i, c);
  set_char(2*i + 1, ((bg & 0x0F) << 4) | (fg & 0x0F));
}

void pad(pos_t cursor_pos, pos_t to_pad)
{
  for(int i = cursor_pos; i < to_pad; i++) {
    put_char(i, ' ', foreground, background);
  }
}

void clear()
{
  pad(0, SCREEN_WIDTH * SCREEN_HEIGHT);
  set_cursor_pos(0);
}


void scroll()
{
  pos_t last_line = SCREEN_WIDTH * (SCREEN_HEIGHT - 1);
  for(pos_t i = 0; i < 2*last_line; i++) {
    set_char(i, get_char(i + 2*SCREEN_WIDTH));
  }

  /* Pad the new line */
  pad(last_line, last_line + SCREEN_WIDTH);
}

void write_box(pos_t upper_left, pos_t lower_right)
{
  /* Beware: upper is smaller coordinates than lower! */
  int upper_row = ROW(upper_left), lower_row = ROW(lower_right);
  int left_col  = COL(upper_left), right_col = COL(lower_right);

  /* kloug(100, "%d %d %d %d %d %d\n", upper_left, lower_right, upper_row, lower_row, left_col, right_col); */

  for (int row = lower_row; row >= upper_row; row += upper_row - lower_row) {
    for (int col = left_col + 1; col < right_col; col++) {
      PUT_CHAR(row, col, HORIZONTAL_BAR);
    }
  }

  for (int col = left_col; col <= right_col; col += right_col - left_col) {
    for (int row = lower_row + 1; row > upper_row; row--) {
      PUT_CHAR(row, col, VERTICAL_BAR);
    }
  }

  PUT_CHAR(upper_row, left_col,  UPPER_LEFT_CORNER);
  PUT_CHAR(upper_row, right_col, UPPER_RIGHT_CORNER);
  PUT_CHAR(lower_row, left_col,  LOWER_LEFT_CORNER);
  PUT_CHAR(lower_row, right_col, LOWER_RIGHT_CORNER);
}

