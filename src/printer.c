#include "printer.h"
#include "io.h"


char * const framebuffer = (char *)FRAMEBUFFER_LOCATION;

void put_char(pos_t i, char c, color_t fg, color_t bg)
{
  // Little endian!
  framebuffer[i] = c;
  framebuffer[i+1] = ((bg & 0x0F) << 4) | (fg & 0x0F);
}


pos_t get_cursor()
{
  
}

void move_cursor(pos_t pos)
{
  // Sends upper byte of position
  outb(COMMAND_PORT, HIGH_BYTE_COMMAND);
  outb(DATA_PORT,    (pos >> 8) & 0x00FF);

  // Sends lower byte of position
  outb(COMMAND_PORT, LOW_BYTE_COMMAND);
  outb(DATA_PORT,    pos & 0x00FF);
}


unsigned int current_pos = 0;
void write(char *buf, unsigned int len)
{
  for (unsigned int i = 0; i < len; i++)
  {
    char c = buf[i];
    put_char(current_pos, c, White, Black);
    current_pos += 2; // Each character actually takes up 2 chars
  }
}
