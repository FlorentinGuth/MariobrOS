#ifndef LOADER_H
#define LOADER_H

#include "lgdt.h"

typedef struct gdt gdt_t;
struct gdt {
  unsigned int address;
  unsigned short size;
} __attribute__((packed));

#endif
