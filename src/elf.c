#include "elf.h"
#include "types.h"
#include "error.h"
#include "memory.h"

#define MAGIC_NUMBER (256*256*256*0x7F + 256*256*'E' + 256*'L' + 'F')


void check_and_load(void *elf_file)
{
  elf_header_t *elf_header = (elf_header_t *)elf_file;

  /* Checks */
  if (!(elf_header->magic_number == MAGIC_NUMBER &&
        elf_header->nb_bits == 1 &&
        elf_header->endian == 1 &&
        elf_header->elf_type == 2 &&
        elf_header->instruction_set == 3 &&
        elf_header->entry_point == START_OF_USER_CODE)) {
    throw("Invalid elf header");
  }

  for (u_int16 segment_number = 0; segment_number < elf_header->pht_entry_nb; segment_number += 1) {
    program_header_entry_t *segment = (program_header_entry_t *) \
      (elf_file + elf_header->program_header_table + segment_number * elf_header->pht_entry_size);

    if (segment->segment_type == Load) {
      void *address = (void *)segment->segment_virtual_address;
      mem_set (address,                                  0, segment->segment_size_in_memory);
      mem_copy(address, elf_file + segment->segment_offset, segment->segment_size_in_file);
    }
  }
}
