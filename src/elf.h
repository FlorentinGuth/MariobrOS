#ifndef ELF_H
#define ELF_H

#include "types.h"


typedef struct elf_header {
  u_int32 magic_number; /* Must be 0x7F then 'ELF' in ASCII */
  u_int8  nb_bits;      /* 1 for 32 bits, 2 or 64 bits */
  u_int8  endian;       /* 1 for little endian, 2 for big endian */
  u_int8  elf_version;
  u_int8  os_abi;
  u_int8  unused[8];
  u_int16 elf_type;        /* 1 for relocatable, 2 for executable, 3 for shared and 4 for core */
  u_int16 instruction_set; /* 3 for x86 */
  u_int32 elf_version2;
  u_int32 entry_point;
  u_int32 program_header_table;  /* Abbreviated as pht */
  u_int32 section_header_table;  /* Abbreviated as sht */
  u_int32 flags;
  u_int16 header_size;
  u_int16 pht_entry_size;
  u_int16 pht_entry_nb;
  u_int16 sht_entry_size;
  u_int16 sht_entry_nb;
  u_int16 section_names_index;
} __attribute__((packed)) elf_header_t;


typedef struct program_header_entry {
  u_int32 segment_type;
  u_int32 segment_offset;
  u_int32 segment_virtual_address;
  u_int32 unused;
  u_int32 segment_size_in_file;
  u_int32 segment_size_in_memory;
  u_int32 flags;
  u_int32 alignment;
} __attribute__((packed)) program_header_entry_t;

typedef enum segment_type {
  Null = 0,         /* To ignore */
  Load = 1,         /* Clear segment_size_in_memory bytes at segment_virtual address,
                     * then copy segment_size_in_file bytes from segment_offset to
                     * segment_virtual_address - This deals with uninitialized data */
  Dynamic = 2,      /* Requires dynamic linking */
  Interpreter = 3,  /* File path to an executable used as an interpreter */
  Note = 4,         /* Note section, ignored here */
} segment_type_t;


typedef struct section_header_entry {
  u_int32 section_name;  /* Index in the string table */
  u_int32 section_type;
  u_int32 flags;
  u_int32 section_virtual_address;  /* Where the first byte of the section should be in memory */
  u_int32 section_offset;
  u_int32 section_size;
  u_int32 link;
  u_int32 info;
  u_int32 alignment;
  u_int32 section_entry_size;
} __attribute__((packed)) section_header_entry_t;

typedef enum section_type {
  NullSection = 0,  /* To ignore */
  ProgBits = 1,
  SymbolTable = 2,
  StringTable = 3,
  RelocationAddends = 4,
  SymbolHashTable = 5,
  DynamicLinking = 6,
  NoteSection = 7,
  NoBits = 8,
  RelocationNoAddends = 9,
  Reserved = 10,
  DynamicSymbolTable = 11,
}section_type_t;

/* Special sections:
 * .bss      - Data that should be initialized with zeros at start-up
 * .data     - Initialized data
 * .fini     - Code that should be run if the program exits normally (ignored for now)
 * .init     - Code that should be run before calling main (ignored for now)
 * .rodata   - Read-only data (should be paged as non-writable)
 * .shstrtab - Holds section names
 * .text     - Executable instructions
 */


/**
 * @name check_and_load - Checks if the ELF file is valid, and loads everything in memory
 * The current page directory must be the process', to be able to reach the virtual load address
 * @param elf_file      - A pointer towards the ELF file, loaded in memory
 * @return              - The entry point address
 */
u_int32 check_and_load(void *elf_file);



#endif
