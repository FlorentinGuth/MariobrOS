#ifndef SYSCALL_ASM_H
#define SYSCALL_ASM_H


/**
 * @name common_interrupt_handler - Same as the one for ISRs and IRQs
 * This will call syscall_handler with a regs_t* argument.
 * @return void
 */
void common_interrupt_handler();

#endif
