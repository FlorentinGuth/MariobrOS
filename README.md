# MariobrOS #

Currently following [James Molloy's tutorial](www.jamesmolloy.co.uk/tutorial_html/6.-Paging.html) and [The OSDev wiki](wiki.osdev.org)

Packages used: bochs (bochs-sdl) bochs-x genisoimage nasm build-essential

### How to run: ###
* **make** or **make runq** will run qemu
* **make disk** or **make diskq** will create a hard disk and boot with qemu on it. It will need superuser rights
* **make runb** and **make diskb** : same as before, but will run bochs instead of qemu. You may have to type c in the console to tell it to continue
* **make clean** will erase compiled files
* **make cleandisk** will also erase the produced disk
* **make mrproper** will reset the directory, erasing configuration files and the reference disk


### Contents: ###
- *build*:    intermediary files
- *emu*:      config for bochs, log for both bochs and qemu
- *doc*:      tutorials used (useful when offline)
- *iso*:      the contents of the iso
- *progs*:    the user programs (sources)
- *src*:      the sources
- *disk*:     hard disk
