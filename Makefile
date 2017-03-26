all:	iso
  # Let's run it!
	bochs -f bochs/config.txt

iso:	kernel.elf
  # Builds the iso image from the iso folder
	genisoimage -R \
							-b boot/grub/stage2_eltorito \
							-no-emul-boot \
							-boot-load-size 4 \
							-A MariobrOS \
							-input-charset utf8 \
							-quiet \
							-boot-info-table \
							-o build/os.iso \
							iso

kernel.elf: 	loader.o
  # Links the file and produces the .elf in iso folder
	ld -T src/link.ld -melf_i386 build/loader.o -o iso/boot/kernel.elf

loader.o:
  # Compiles the loader into ELF 32 bits object file loader.o
	nasm -f elf32 src/loader.s -o build/loader.o

clean:
	rm -rf build/**
	rm -f isoboot/kernel.elf
