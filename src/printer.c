#include "printer.h"


char * const framebuffer = (char *)FRAMEBUFFER_LOCATION;


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


void pad(pos_t cursor_pos, pos_t to_pad)
{
  for(int i=cursor_pos; i<to_pad; i++) {
    put_char(i, ' ', White, Black);
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


void write_char(const char c)
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
    pad(cursor_pos+1, to_pad);
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
    put_char(cursor_pos, ' ', White, Black);
    break;
  }

  case '\177' : { /* delete*/
    for (int i = 2*cursor_pos; i < 2 * SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
      framebuffer[i] = framebuffer[i+2];
    }
    break;
  }
    
  default: {
    put_char(cursor_pos, c, White, Black);
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

void write(const char *string)
{
  for (unsigned int i = 0;; i++) {
    char c = string[i];
    if(c=='\0')
      return;
    else
      write_char(c);
  }
}

void write_int(const int n)
{
  char buf[20];
  to_string(buf,n);
  write(buf);
}
