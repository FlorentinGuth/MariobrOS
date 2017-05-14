#include "printer.h"
#include "error.h"
#include "logging.h"
#include "utils.h"


char * const framebuffer = (char *)FRAMEBUFFER_LOCATION;

unsigned char utf8_c2[] = {173,155,156,234,157,179,21,234,234,166,234,170,174,234,234,248,241,253,234,'\'',230,20,249,',','1',167,175,172,171,234,168};
unsigned char utf8_c3[] = {'A','A','A','A',142,143,146,128,'E',144,'E','E','I','I','I','I','D',165,'O','O','O','O',153,'x','O','U','U','U',154,'Y','0',225,133,160,131,'a',132,134,145,135,138,130,136,137,141,161,140,139,235,164,149,162,147,'o',148,246,237,151,163,150,129,'y',0,152};


void put_char(pos_t i, char c, color_t fg, color_t bg)
{
  /* Each character actually takes up two bytes */
  framebuffer[2*i] = c;
  framebuffer[2*i + 1] = ((bg & 0x0F) << 4) | (fg & 0x0F);
}


pos_t get_cursor_pos()
{
  /* See http://www.osdever.net/FreeVGA/vga/vgareg.htm#indexed for reference */

  /* Saves the content of the address register (may be useless...) */
  unsigned char temp = inb(ADDRESS_REG);

  /* Requests the higher byte of position */
  outb(ADDRESS_REG, CURSOR_HIGH_BYTE);
  unsigned char higher_byte = inb(DATA_REG);

  /* Requests the lower byte of position */
  outb(ADDRESS_REG, CURSOR_LOW_BYTE);
  unsigned char lower_byte = inb(DATA_REG);

  /* Restores the address register */
  outb(ADDRESS_REG, temp);

  return (higher_byte << 8) | lower_byte;
}

void set_cursor_pos(pos_t pos)
{
  /* Sends higher byte of position */
  outb(ADDRESS_REG, CURSOR_HIGH_BYTE);
  outb(DATA_REG,    (pos >> 8) & 0x00FF);

  /* Sends lower byte of position */
  outb(ADDRESS_REG, CURSOR_LOW_BYTE);
  outb(DATA_REG,    pos & 0x00FF);
}


/* The colors used in the shell */
color_t foreground = White;
color_t background = Black;


void pad(pos_t cursor_pos, pos_t to_pad)
{
  for(int i=cursor_pos; i<to_pad; i++) {
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
  for(int i=0; i < 2*last_line; i++) {
    framebuffer[i] = framebuffer[i + 2*SCREEN_WIDTH];
  }

  /* Pad the new line */
  pad(last_line, last_line + SCREEN_WIDTH);
}


void write_char(char c)
{
  pos_t cursor_pos = get_cursor_pos();
  switch (c) {

  case '\0': {
    return;
    break;
  }

  case '\n': {
    /* It should be useless to pad the spaces, but we keep it in case the user
       messes with the framebuffer by writing everywhere */
    pos_t to_pad = SCREEN_WIDTH * ((cursor_pos / SCREEN_WIDTH) + 1);
    pad(cursor_pos, to_pad);
    cursor_pos = to_pad;
    break;
  }

  case '\t': {
    /* Same as '\n' */
    pos_t to_pad = TAB_WIDTH * ((cursor_pos / TAB_WIDTH) + 1);
    pad(cursor_pos, to_pad);
    cursor_pos = to_pad;
    break;
  }

  case '\b' : { /* backspace */
    cursor_pos--;
    put_char(cursor_pos, ' ', foreground, background);
    break;
  }

  case '\177' : { /* delete*/
    for (int i = 2*cursor_pos; i < 2 * SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
      framebuffer[i] = framebuffer[i+2];
    }
    break;
  }

  default: {
    put_char(cursor_pos, c, foreground, background);
    cursor_pos++;
  }
  }

  /* Check if scrolling would be necessary
     TODO: maybe pre-compute the number of lines to scroll to do it only once */
  if (cursor_pos == SCREEN_WIDTH * SCREEN_HEIGHT) {
    scroll();
    cursor_pos -= SCREEN_WIDTH;
  }

  set_cursor_pos(cursor_pos);
}

void write_string(const char *string)
{
  for (unsigned int i = 0;; i++) {
    char c = string[i];
    if(c=='\0')
      return;
    else if(c==0xc2) {
      i++;
      write_char(utf8_c2[(unsigned int)(string[i]-0xa1)]);
    }
    else if(c==0xc3) {
      i++;
      write_char(utf8_c3[(unsigned int)(string[i]-0x80)]);
    }
    else
      write_char(c);
  }
}

void write_int(int n)
{
  int len = int_to_string(0, n, 10);
  char buf[len];
  int_to_string(buf, n, 10);
  write_string(buf);
}

void write_hex(int n)
{
  int len = u_int_to_string(0, n, 16);
  char buf[len];
  u_int_to_string(buf, n, 16);
  write_string("0x");
  write_string(buf);
}

void writef(char s[], ...)
{
  va_list param;
  va_start(param, 0);

  int read  = 0;
  char buffer[17];
  char c = s[0];

  while(c!='\0') {
    if(c=='%') {
      read++;
      switch(s[read]) {

      case 'd': { // Decimal (signed)
        int_to_string(buffer, va_arg(param,int), 10);
        write_string(buffer); break; }
      case 'u': { // Decimal (unsigned)
        u_int_to_string(buffer, va_arg(param,unsigned int), 10);
        write_string(buffer); break; }
      case 'x': { // Hexadecimal
        u_int_to_string(buffer, va_arg(param,int), 16);
        write_string("0x"); write_string(buffer); break; }
      case 'X': {
        u_int32 hex = va_arg(param,u_int32);
        u_int32 length = u_int_to_string(0, hex, 16);
        u_int32 min_length = va_arg(param, u_int32);
        write_string("0x");
        for (unsigned int i = 0; i < max(0, min_length - length); i++) {  /* FIXME: max signed... */
          write_char('0');
        }
        u_int_to_string(buffer, hex, 16);
        write_string(buffer); break;
      }
      case 'h': { // Hexadecimal (without "0x")
        u_int_to_string(buffer, va_arg(param,int), 16);
        write_string(buffer); break; }
      case 'c': { // Character
        write_char(va_arg(param,int)); break; }
      case 's': { // String
        write_string(va_arg(param,string)); break; }
      case 'f': { // Foreground color
        foreground = va_arg(param, color_t); break; }
      case 'b': { // Background color
        background = va_arg(param, color_t); break; }
      case '%': { // Writes a '%'
        write_char('%'); break; }
      default:  { // Emergency stop
        throw("Invalid format string"); }
      }
    } else if(c==0xc2) {
      read++;
      write_char(utf8_c2[(unsigned int)(s[read]-0xa1)]);
    }
    else if(c==0xc3) {
      read++;
      write_char(utf8_c3[(unsigned int)(s[read]-0x80)]);
    }
    else
      write_char(c);
    read++;
    c = s[read];
  }
  va_end(param);
}

#define VERTICAL_BAR       186
#define UPPER_RIGHT_CORNER 187
#define LOWER_RIGHT_CORNER 188
#define LOWER_LEFT_CORNER  200
#define UPPER_LEFT_CORNER  201
#define HORIZONTAL_BAR     205

#define PUT_CHAR(row, col, c) put_char(POS(row, col), (char)c, foreground, background)

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
