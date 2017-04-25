OS_NAME = MariobrOS

# Options

GUI = x
GUEST_MEMORY = 512  # Memory that our OS should have
HOST_MEMORY  = 512  # Memory that bochs should use to emulate our OS

# Folders and paths
SRC_DIR   = src
BUILD_DIR = build
ISO_DIR   = iso
BOCHS_DIR = bochs

DISK_DIR  = disk
LOOP_DEVICE = /dev/loop0
MNT_DIR = /mnt/$(OS_NAME)_tmp
DISK_IMG = $(DISK_DIR)/disk.img

# File names
LINKER = $(SRC_DIR)/link.ld
OBJECTS = loader.o kmain.o malloc.o paging.o kheap.o memory.o ata_pio.o gdt.o gdt_asm.o timer.o keyboard.o irq.o irq_asm.o isr.o isr_asm.o idt.o idt_asm.o logging.o printer.o string.o io.o math.o
OBJS = $(addprefix $(BUILD_DIR)/,$(OBJECTS))
OS_ISO = $(BUILD_DIR)/os.iso
KERNEL_ELF = $(ISO_DIR)/boot/kernel.elf

BOCHS_CONFIG_CD = config_cd.txt
BOCHS_CONFIG_DISK = config_disk.txt
BOCHS_LOG = log.txt
GRUB_CONFIG = $(ISO_DIR)/boot/grub/menu.lst
GRUB2_CONFIG = $(ISO_DIR)/boot/grub/grub.cfg
ELTORITO = boot/grub/stage2_eltorito

# Configuration files

define BOCHS_CONFIG_CONTENT
megs:             32
display_library:  $(GUI)
romimage:         file=/usr/share/bochs/BIOS-bochs-latest
vgaromimage:      file=/usr/share/bochs/VGABIOS-lgpl-latest
log:              bochs/$(BOCHS_LOG)
clock:            sync=none, time0=local
cpu:              count=1, ips=1000000
memory:           guest=$(GUEST_MEMORY), host=$(HOST_MEMORY)
com1:             enabled=1, mode=file, dev=$(BOCHS_DIR)/com1.out
keyboard:         type=mf, serial_delay=200, paste_delay=100000, keymap=$(SRC_DIR)/x11-pc-fr.map
endef

define BOCHS_CONFIG_BOOT_CD
ata0-master:      type=cdrom, path=$(OS_ISO), status=inserted
boot:             cdrom
endef
define BOCHS_CONFIG_BOOT_DISK
ata0:             enabled=1, ioaddr1=0x1f0, ioaddr2=0x3f0, irq=14
ata0-master:      type=disk, path=$(DISK_IMG), mode=flat, translation=auto
boot:             disk
endef

define GRUB_CONFIG_CONTENT
default=0
timeout=0

title $(OS_NAME)
kernel /boot/kernel.elf
endef

define GRUB2_CONFIG_CONTENT
set timeout=0
menuentry "$(OS_NAME)" {
	multiboot /boot/kernel.elf
}
endef

define NEWLINE


endef

# C compiler and flags
CC =     gcc
CFLAGS =-m32 \
        -nostdlib -nodefaultlibs \
        -ffreestanding \
        -fno-builtin -nostartfiles\
        -fno-stack-protector \
        -funsigned-char -funsigned-bitfields \
        -Wall -Wextra -Werror \
        -O0 \
        -c
# Compiles in 32 bits mode, without any std, with all warnings (and treating warning as errors) \
  and with no linking (and disable optimizations)

# Linker and flags
LD =      ld
LDFLAGS = -T $(LINKER)

# ASM compiler and flags
AS =      nasm
ASFLAGS = -f elf



all: run


.PHONY: all run rundisk syncdisk disk clean cleandisk


log: run
	cat $(BOCHS_DIR)/$(BOCHS_LOG)


$(BUILD_DIR) $(BOCHS_DIR): # Ensures folders exist
	mkdir -p $@

# Configuration files writing
ECHO_CONFIG = @echo '$(subst $(NEWLINE),\n,$(1))' > $(2)

$(BOCHS_CONFIG_CD): $(BOCHS_DIR)
	$(call ECHO_CONFIG,$(BOCHS_CONFIG_CONTENT)$(NEWLINE)$(BOCHS_CONFIG_BOOT_CD),$(BOCHS_DIR)/$(BOCHS_CONFIG_CD))
$(BOCHS_CONFIG_DISK): $(BOCHS_DIR)
	$(call ECHO_CONFIG,$(BOCHS_CONFIG_CONTENT)$(NEWLINE)$(BOCHS_CONFIG_BOOT_DISK),$(BOCHS_DIR)/$(BOCHS_CONFIG_DISK))

$(GRUB_CONFIG):
	$(call ECHO_CONFIG,$(GRUB_CONFIG_CONTENT),$(GRUB_CONFIG))
$(GRUB2_CONFIG):
	$(call ECHO_CONFIG,$(GRUB2_CONFIG_CONTENT),$(GRUB2_CONFIG))




run: $(OS_ISO) $(BOCHS_CONFIG_CD)
	bochs -q -f $(BOCHS_DIR)/$(BOCHS_CONFIG_CD)

disk: syncdisk
	bochs -q -f $(BOCHS_DIR)/$(BOCHS_CONFIG_DISK)

qemu: $(OS_ISO) $(BOCHS_CONFIG_CD)
	qemu-system-i386 -cdrom build/os.iso

diskqemu: syncdisk
	qemu-system-i386 -boot c -drive format=raw,file=$(DISK_IMG) -m 512 -s

$(BUILD_DIR)/%.o: src/%.c $(BUILD_DIR)
	@$(CC) $< -c -o $@  $(CFLAGS) $(CPPFLAGS)

$(BUILD_DIR)/%.o: src/%.s $(BUILD_DIR)
	@$(AS) $< -o $@ $(ASFLAGS)

$(OS_ISO):	$(KERNEL_ELF) $(GRUB_CONFIG)
    # Builds the ISO image from the ISO folder
	@genisoimage -R \
                -b $(ELTORITO) \
                -no-emul-boot \
                -boot-load-size 4 \
                -A $(OS_NAME) \
                -input-charset utf8 \
                -quiet \
                -boot-info-table \
                -o $(OS_ISO) \
                iso

$(KERNEL_ELF):	$(OBJS)
    # Links the file and produces the .elf in the ISO folder
	$(LD) $(LDFLAGS) $(OBJS) -o $(KERNEL_ELF) -melf_i386


$(DISK_IMG):
	@mkdir -p $(DISK_DIR)
	dd if=/dev/zero of=$(DISK_IMG) bs=512 count=131072
	@sudo losetup $(LOOP_DEVICE) $(DISK_IMG)
	@sudo echo "," | sfdisk --quiet disk/disk.img
	@sudo partprobe $(LOOP_DEVICE)
	@sudo mkfs.ext2 -q -b 2048 $(LOOP_DEVICE)p1
	@sudo mkdir -p $(MNT_DIR)
	@sudo mount $(LOOP_DEVICE)p1 $(MNT_DIR)
	sudo grub-install --root-directory=$(MNT_DIR) --no-floppy --modules="normal part_msdos ext2 multiboot" --target=i386-pc $(LOOP_DEVICE)
	@sudo umount -d $(MNT_DIR)
	@sudo rm -rf $(MNT_DIR)
	@sudo losetup -d $(LOOP_DEVICE)

syncdisk: $(DISK_IMG) $(GRUB2_CONFIG) $(BOCHS_CONFIG_DISK) $(KERNEL_ELF)
	@sudo losetup $(LOOP_DEVICE) $(DISK_IMG)
	@sudo partprobe $(LOOP_DEVICE)
	@sudo mkdir -p $(MNT_DIR)
	@sudo mount $(LOOP_DEVICE)p1 $(MNT_DIR)
	sudo rsync -r $(ISO_DIR)/boot $(MNT_DIR)
	@sudo umount -d $(MNT_DIR)
	@sudo rm -rf $(MNT_DIR)
	@sudo losetup -d $(LOOP_DEVICE)

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(BOCHS_DIR)
	rm -f $(ISO_DIR)/boot/kernel.elf
	rm -f $(GRUB_CONFIG)
	rm -f $(GRUB2_CONFIG)

cleandisk: clean
	rm -rf disk
