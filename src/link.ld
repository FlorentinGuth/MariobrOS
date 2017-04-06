ENTRY(loader)               /* The name of the entry label */

SECTIONS {
  /* Space reserved for the BIOS and GRUB */

  . = 0x00100000;           /* The code should be loaded at 1 MB */

  .text ALIGN (0x1000) :    /* Align at 4KB */
  {
    *(.text)                /* All text sections from all files */
  }

  .rodata ALIGN (0x1000) :  /* Align at 4KB */
  {
    *(.rodata)              /* All read-only data sections from all files */
  }

  .data ALIGN (0x1000) :    /* Align at 4 KB */
  {
    *(.data)                /* All data sections from all files */
  }

  .bss ALIGN (0x1000) :     /* Align at 4 KB */
  {
    *(COMMON)               /* All COMMON sections from all files */
    *(.bss)                 /* All bss (Block Started by Symbol) sections from all files */
  }

  end_of_kernel_space = .;  /* Declares a variable whose ADDRESS is the end of the kernel-reserved space */

  . = 0x00400000;           /* The memory should be free after this point (4MB) */
}
