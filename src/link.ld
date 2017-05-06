ENTRY(loader)               /* The name of the entry label */

/* Every section is page-aligned (4KB, i.e. 0x1000) */
SECTIONS {
  /* Space reserved for the BIOS and GRUB */

  . = 0x00100000;           /* The code should be loaded at 1 MB */
  ld_start = .;             /* Declares a variable whose ADDRESS is the start of the kernel space */

  .text ALIGN (0x1000) :    /* Align at 4KB */
  {
    ld_code = .;            /* Declares a variable whose ADDRESS is the start of the code */
    *(.text)                /* All text sections from all files */
  }

  .data ALIGN (0x1000) :    /* Align at 4 KB */
  {
    ld_data = .;            /* Declares a variable whose ADDRESS is the start of the data section */
    *(.data)                /* All data sections from all files */
    *(.rodata)              /* All read-only data sections from all files */
  }

  .bss ALIGN (0x1000) :     /* Align at 4 KB */
  {
    ld_bss = .;             /* Declares a variable whose ADDRESS is the start of the bss section */
    *(.bss)                 /* All bss (Block Started by Symbol) sections from all files */
    *(COMMON)               /* All COMMON sections from all files */
  }

  ld_end = .;               /* Declares a variable whose ADDRESS is the end of the kernel-reserved space */
}
