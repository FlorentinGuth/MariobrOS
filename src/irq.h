#ifndef IRQ_H
#define IRQ_H

#include "isr.h"

/* irq_install_handler 
 * @param irq      The number of the irq for which to install the handler
 * @param handler  The handler to install
*/
void irq_install_handler(int irq, void (*handler)(struct regs *r));


/* irq_uninstall_handler:
 * @param irq     The number of the irq for which to uninstall the handler */
void irq_uninstall_handler(int irq);

/* irq_install
 * Sets the irq handlers */
void irq_install();

/* irq_handler
 * @param r       Wrapper for the registers' state */
void irq_handler(struct regs *r);

#endif