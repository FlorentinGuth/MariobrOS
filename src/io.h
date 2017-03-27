#ifndef IO_H
#define IO_H


/** outb:
 *  Sends data to an I/O port.
 *
 *  @param port The I/O port to send the data to
 *  @param data The data to send to the I/O port
 */
void outb(unsigned short port, unsigned char data);

#endif
