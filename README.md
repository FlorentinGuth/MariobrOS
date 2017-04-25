# MariobrOS #

Currently following [James Molloy's tutorial](www.jamesmolloy.co.uk/tutorial_html/6.-Paging.html) and [The OSDev wiki](wiki.osdev.org)

Packages used: bochs (bochs-sdl) bochs-x genisoimage nasm build-essential

### How to run: ###
* **make** will run bochs, you may have to type c in the console to tell it to continue
* **make disk** will create a hard disk and boot with bochs on it. It will need superuser rights
* **make qemu** and **make diskqemu** : same as before, but run qemu instead of bochs.

### Contents: ###
- *build*:    intermediary files
- *bochs*:    config, logs, etc for bochs
- *doc*:      tutorials used (useful when offline)
- *iso*:      the contents of the iso
- *kernel*:   legacy C kernel (unused yet)
- *src*:      the sources
- *disk*:     hard disk
