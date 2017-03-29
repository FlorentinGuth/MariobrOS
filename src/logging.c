#include "types.h"
#include "logging.h"
#include "io.h"
#include "string.h"
#include "printer.h"

void serial_configure_baud_rate(port_t com, port_t divisor)
{
  outb(SERIAL_LINE_COMMAND_PORT(com), SERIAL_LINE_ENABLE_DLAB);
  outb(SERIAL_DATA_PORT(com),         (divisor >> 8) & 0x00FF);
  outb(SERIAL_DATA_PORT(com),          divisor       & 0x00FF);
}

void serial_configure_line(port_t com)
{
  /* Bit:     | 7 | 6 | 5 4 3 | 2 | 1 0 |
   * Content: | d | b | prty  | s | dl  |
   * Value:   | 0 | 0 | 0 0 0 | 0 | 1 1 | = 0x03
   */
  outb(SERIAL_LINE_COMMAND_PORT(com), 0x03);
}

void serial_configure_buffers(port_t com)
{
  /* Bit:     | 7 6 | 5  | 4 |  3  |  2  |  1  | 0 |
   * Content: | lvl | bs | r | dma | clt | clr | e |
   * Value:   | 1 1 | 1  | 0 |  0  |  1  |  1  | 1 | = 0xE7
   */
  outb(SERIAL_FIFO_COMMAND_PORT(com), 0xE7);
}

void serial_configure_modem(port_t com)
{
  /* Bit:     | 7 | 6 | 5  | 4  |  3  |  2  |  1  |  0  |
   * Content: | r | r | af | lb | ao2 | ao1 | rts | dtr |
   * Value:   | 0 | 0 | 0  | 0  |  0  |  0  |  1  |  1  | = 0x03
   */
  outb(SERIAL_MODEM_COMMAND_PORT(com), 0x03);
}


bool serial_is_transmit_fifo_empty(port_t com)
{
  /* 0x20 = 0010 0000 */
  return inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
}

bool configured = FALSE;
void log(char *string, log_level_t lvl)
{
  if (!configured) {
    serial_configure_baud_rate(SERIAL_COM1_BASE, 1);
    serial_configure_line(SERIAL_COM1_BASE);
    serial_configure_buffers(SERIAL_COM1_BASE);
    serial_configure_modem(SERIAL_COM1_BASE);

    configured= TRUE;
  }

  char level[9];
  switch (lvl) {
  case Debug: {
    str_copy("[DEBUG] ", level);
    break;
  }
  case Info: {
    str_copy("[INFO]  ", level);
    break;
  }
  case Error: {
    str_copy("[ERROR] ", level);
    break;
  }
  }

  for (int i = 0; level[i] != '\0'; i++) {
    outb(SERIAL_COM1_BASE, level[i]);
  }
  for (int i = 0; string[i] != '\0'; i++) {
    outb(SERIAL_COM1_BASE, string[i]);
    write(""); // Truly terrible hack used to slow down the output pace
  }
}
