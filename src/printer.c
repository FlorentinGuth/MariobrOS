#include "types.h"
#include "printer.h"
#include "io.h"


char * const framebuffer = (char *)FRAMEBUFFER_LOCATION;

void put_char(pos_t i, char c, color_t fg, color_t bg)
{
  framebuffer[i] = c;
  framebuffer[i+1] = ((bg & 0x0F) << 4) | (fg & 0x0F);
}


pos_t get_cursor_pos()
{
  // See http://www.osdever.net/FreeVGA/vga/vgareg.htm#indexed for reference

  // Saves the content of the address register
  unsigned char temp = inb(ADDRESS_REG);

  // Requests the higher byte of position
  outb(ADDRESS_REG, CURSOR_HIGH_BYTE);
  unsigned char higher_byte = inb(DATA_REG);

  // Requests the lower byte of position
  outb(ADDRESS_REG, CURSOR_LOW_BYTE);
  unsigned char lower_byte = inb(DATA_REG);

  // Restores the address register
  outb(ADDRESS_REG, temp);

  return (higher_byte << 8) | lower_byte;
}

void set_cursor_pos(pos_t pos)
{
  // Sends higher byte of position
  outb(ADDRESS_REG, CURSOR_HIGH_BYTE);
  outb(DATA_REG,    (pos >> 8) & 0x00FF);

  // Sends lower byte of position
  outb(ADDRESS_REG, CURSOR_LOW_BYTE);
  outb(DATA_REG,    pos & 0x00FF);
}


void write(char *string)
{
  pos_t cursor_pos = get_cursor_pos();

  bool done = FALSE;
  for (unsigned int i = 0; !done; i++) {

    char c = string[i];

    switch (c) {

    case '\0': {
      done = TRUE;
      break;
    }

    case '\n': {
      cursor_pos = SCREEN_WIDTH * ((cursor_pos / SCREEN_WIDTH) + 1);
      break;
    }

    case '\t': {
      cursor_pos = TAB_WIDTH * ((cursor_pos / TAB_WIDTH) + 1);
      break;
    }

    default: {
      put_char(2*cursor_pos, c, White, Black); // Each character actually takes up 2 chars
      cursor_pos++;
    }
    }

    // TODO: deal with scrolling
  }

  set_cursor_pos(cursor_pos);
}
