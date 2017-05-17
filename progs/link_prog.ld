ENTRY(loader)               /* Our custom entry point */

/* Every section is page-aligned (4KB, i.e. 0x1000) */
SECTIONS {
  /* Space reserved for the kernel */

  . = 0xFFFF0000;           /* The code should be loaded at 4GB - 64KB */

  .text ALIGN (0x1000) :    /* Align at 4KB */
  {
    *(.text)                /* All text sections from all files */
  }

  .data ALIGN (0x1000) :    /* Align at 4 KB */
  {
    *(.data)                /* All data sections from all files */
    *(.rodata)              /* All read-only data sections from all files */
  }

  .bss ALIGN (0x1000) :     /* Align at 4 KB */
  {
    *(.bss)                 /* All bss (Block Started by Symbol) sections from all files */
    *(COMMON)               /* All COMMON sections from all files */
  }
}
