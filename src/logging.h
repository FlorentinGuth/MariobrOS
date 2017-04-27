#ifndef LOGGING_H
#define LOGGING_H

/** logging.h:
 *  Provides utilities for logging with Bochs, through a serial port.
 */

#include "printer.h"

/** The I/O ports:
 *  All the I/O ports are calculated relative to the data port. This is because
 *  all serial ports (COM1, COM2, COM3, COM4) have their ports in the same
 *  order, but they start at different values.
 */
#define SERIAL_COM1_BASE                (port_t)0x3F8      /* COM1 base port */

#define SERIAL_DATA_PORT(base)          (port_t)(base)
#define SERIAL_FIFO_COMMAND_PORT(base)  (port_t)(base + 2)
#define SERIAL_LINE_COMMAND_PORT(base)  (port_t)(base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) (port_t)(base + 4)
#define SERIAL_LINE_STATUS_PORT(base)   (port_t)(base + 5)


/** The I/O port commands */

/** SERIAL_LINE_ENABLE_DLAB:
 *  Tells the serial port to expect first the highest 8 bits on the data port,
 *  then the lowest 8 bits will follow
 */
#define SERIAL_LINE_ENABLE_DLAB         0x80


/** serial_configure_baud_rate:
 *  Sets the speed of the data being sent. The default speed of a serial
 *  port is 115200 bits/s. The argument is a divisor of that number, hence
 *  the resulting speed becomes (115200 / divisor) bits/s.
 *
 *  @param com     The serial port to configure
 *  @param divisor The divisor
 */
void serial_configure_baud_rate(port_t com, unsigned short divisor);

/** serial_configure_line:
 * Configures the line of the given serial port. The port is set to have a
 * data length of 8 bits, no parity bits, one stop bit and break control
 * disabled.
 *
 * @param com The serial port to configure
 */
void serial_configure_line(port_t com);

/** serial_configure_buffers:
 *  Configures the buffers (in and out) of the given serial port. The buffers are
 *  set in FIFO mode, a queue of 64 bytes and we clear them.
 *
 *  @param com The serial port to configure
 */
void serial_configure_buffers(port_t com);

/** serial_configure_modem:
 *  Configures the modem of the given serial port. It is set to be ready to
 *  transmit data.
 *
 * @param com The serial port to configure
 */
void serial_configure_modem(port_t com);


/** serial_is_transmit_fifo_empty:
 *  Checks whether the transmit FIFO queue is empty or not for the given COM
 *  port.
 *
 * @param com The COM port
 * @return    Whether the FIFO queue is empty or not
 */
bool serial_is_transmit_fifo_empty(port_t com);


enum LogLevel { Debug, Info, Error };
typedef enum LogLevel log_level_t;
/** log:
 *  Sends a string to the COM1 port, i.e. to the com1.out file thanks to Bochs.
 *
 *  @param string The string to send
 *  @param lvl    The severeness of the message
 */
void log_string(const char *string, log_level_t lvl);


/** kloug:
 *  Sens a formatted string to the COM1 port.
 *
 *  @param size    Expected string size after deformatting
 *  @param s       Formatted string
 *  @param ...     Arguments of the formatted string
 */
void kloug(const int size, char s[], ...);

#endif
