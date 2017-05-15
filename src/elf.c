#include "elf.h"
#include "types.h"
#include "error.h"
#include "memory.h"
#include "logging.h"


/* Translates a process' virtual address into a kernel's virtual address */
#define ADR(v)  (u_int8 *)(virtuals[(v-START_OF_USER_CODE) / 0x1000] + (v-START_OF_USER_CODE) % 0x1000)

void mem_set_page(void *str, char c, u_int32 length, u_int32 virtuals[])
{
  for (u_int32 i = 0; i < length; i++) {
    *ADR((u_int32)str + i) = c;
  }
}

void mem_copy_page(void *dest, const void *src, u_int32 length, u_int32 virtuals[])
{
  for (u_int32 i = 0; i < length; i++) {
    *ADR((u_int32)dest + i) = *((u_int8 *)src + i);
  }
}


u_int32 check_and_load(void *elf_file, u_int32 virtuals[])
{
  /* kloug(100, "Checking and loading ELF\n"); */

  elf_header_t *elf_header = (elf_header_t *)elf_file;

  u_int8 *magic_number = (u_int8 *)&elf_header->magic_number;

  /* Checks */
  if (!(magic_number[0] == 0x7F &&
        magic_number[1] == 'E' &&
        magic_number[2] == 'L' &&
        magic_number[3] == 'F')) {
    throw("Not a ELF file");
  }
  if (!(elf_header->nb_bits == 1)) {
    throw("Not a 32 bits executable");
  }
  if (!(elf_header->endian == 1)) {
    throw("Not a little endian executable");
  }
  if (!(elf_header->elf_type == 2)) {
    throw("Not an executable");
  }
  if (!(elf_header->instruction_set == 3)) {
    throw("Not a x86 executable");
  }
  if (!(elf_header->entry_point >= START_OF_USER_CODE)) {
    writef("Wrong entry point: %x compared to %x\n", elf_header->entry_point, START_OF_USER_CODE);
    throw("Wrong entry point");
  }

  /* kloug(100, "Valid header\n"); */

  for (u_int16 segment_number = 0; segment_number < elf_header->pht_entry_nb; segment_number += 1) {
    program_header_entry_t *segment = (program_header_entry_t *) \
      (elf_file + elf_header->program_header_table + segment_number * elf_header->pht_entry_size);
    /* kloug(100, "Segment %d offset %x\n", segment_number, segment->segment_offset); */
    /* kloug(100, "File at %X and segment at %X\n", elf_file, 8, elf_file + segment->segment_offset, 8); */

    if (segment->segment_type == Load) {
      void *address = (void *)segment->segment_virtual_address;
      /* kloug(100, "Should be loaded at %X (mapped at %X)\n", address, 8, ADR((u_int32)address), 8); */
      /* kloug(100, "Size in mem %x and in file %x\n", \ */
            /* segment->segment_size_in_memory, segment->segment_size_in_file); */
      mem_set_page (address,                                  0, segment->segment_size_in_memory, virtuals);
      mem_copy_page(address, elf_file + segment->segment_offset, segment->segment_size_in_file,   virtuals);
    }
  }

  /* kloug(100, "First instruction %X\n", *(u_int32 *)ADR(elf_header->entry_point), 8); */
  return elf_header->entry_point;
}
