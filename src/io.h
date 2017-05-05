/**
 *  @name io.h - Simple wrapper for assembly primitives 'in' and 'out'.
 */

#ifndef IO_H
#define IO_H

#include "types.h"

/**
 *  @name outb  - Sends data (a byte) to an I/O port.
 *
 *  @param port - The I/O port to send the data to
 *  @param data - The data to send to the I/O port
 */
void outb (u_int16 port, u_int8 data);

void outw (u_int16 port, u_int16 data);

/**
 *  @name inb   - Receives data (a byte) from an I/O port.
 *
 *  @param port - The port to listen to
 *  @return       The data
 */
u_int8 inb (u_int16 port);

u_int16 inw (u_int16 port);

void io_wait();

#endif
