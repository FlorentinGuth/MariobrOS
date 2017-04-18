#ifndef KMAIN_H
#define KMAIN_H

#include "gdt.h"
#include "timer.h"
#include "multiboot.h"
/**
 * @name kmain - The main kernel function
 * @param mbd -  The multiboot header provided by GRUB
 * @return int
 */
int kmain(multiboot_info_t* mbd);

#endif
