#include "segmentation.h"


/* The different segments available */
gdt_t const null        = { .address = 0,    .size = 0 };
gdt_t const kernel_code = { .address = 0x08, .size = 0xFFFFFFFF};
gdt_t const kernel_data = { .address = 0x10, .size = 0xFFFFFFFF};

void load_gdt(gdt_t gdt)
{
  __asm__("lgdt [esp + 4]");
}
