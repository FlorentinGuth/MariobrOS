#ifndef IO_H
#define IO_H


typedef unsigned short port_t;

/** outb:
 *  Sends data (a byte) to an I/O port.
 *
 *  @param port The I/O port to send the data to
 *  @param data The data to send to the I/O port
 */
void outb(port_t port, unsigned char data);

/** inb:
 *  Receives data (a byte) from an I/O port.
 *
 *  @param port The port to listen to
 *  @return The data
 */
port_t inb(port_t port);

#endif
