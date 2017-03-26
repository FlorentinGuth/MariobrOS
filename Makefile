OS_NAME = MariobrOS


# Folders and paths
SRC_FOLDER = src
BUILD_FOLDER = build
ISO_FOLDER = iso
BOCHS_FOLDER = bochs
VPATH = $(SRC_FOLDER) $(BUILD_FOLDER)	$(BOCH_FOLDER) # Helps make find our files

# File names
LINKER = link.ld
OBJECTS = loader.o kmain.o
ISO = os.iso
BOCHS_CONFIG = config.txt
BOCHS_LOG = log.txt
GRUB_CONFIG = $(ISO_FOLDER)/boot/grub/menu.lst
ELTORITO = boot/grub/stage2_eltorito

# C compiler and flags
CC = 			gcc
CFLAGS = 	-m32 \
					-nostdlib \
					-nostdinc \
					-fno-builtin \
					-fno-stack-protector \
					-nostartfiles \
					-nodefaultlibs \
					-Wall -Wextra -Werror \
					-c
# Compiles in 32 bits mode, without any std, with all warnings (and treating warning as errors) \
	and with no linking

# Linker and flags
LD =			ld
LDFLAGS = -T $(SRC_FOLDER)/$(LINKER) \
					-melf_i386

# ASM compiler and flags
AS = 			nasm
ASFLAGS =	-f elf32


# Config files
define BOCHS_CONFIG_CONTENT
megs:             32
display_library:  sdl
romimage:         file=/usr/share/bochs/BIOS-bochs-latest
vgaromimage:      file=/usr/share/bochs/VGABIOS-lgpl-latest
ata0-master:      type=cdrom, path=$(BUILD_FOLDER)/$(ISO), status=inserted
boot:             cdrom
log:              $(BOCHS_FOLDER)/$(BOCHS_LOG)
clock:            sync=realtime, time0=local
cpu:              count=1, ips=1000000
endef

define GRUB_CONFIG_CONTENT
default=0
timeout=0

title $(OS_NAME)
kernel /boot/kernel.elf
endef

define NEWLINE


endef


all:	run

run:	$(ISO) $(BOCHS_CONFIG)
  # Let's run it!
	bochs -f $(BOCHS_FOLDER)/$(BOCHS_CONFIG)

$(ISO):	kernel.elf $(GRUB_CONFIG)
  # Builds the iso image from the iso folder
	genisoimage -R \
							-b $(ELTORITO) \
							-no-emul-boot \
							-boot-load-size 4 \
							-A $(OS_NAME) \
							-input-charset utf8 \
							-quiet \
							-boot-info-table \
							-o $(BUILD_FOLDER)/$(ISO) \
							iso

kernel.elf: 	$(OBJECTS)
  # Links the file and produces the .elf in iso folder
	$(LD) $(LDFLAGS) $(addprefix $(BUILD_FOLDER)/,$(OBJECTS)) -o $(ISO_FOLDER)/boot/kernel.elf

%.o:	%.c
	$(CC) $(CFLAGS) $< -o $(BUILD_FOLDER)/$@

%.o:	%.s
	$(AS) $(ASFLAGS) $< -o $(BUILD_FOLDER)/$@

$(BOCHS_CONFIG):
	echo '$(subst $(NEWLINE),\n,${BOCHS_CONFIG_CONTENT})' > $(BOCHS_FOLDER)/$(BOCHS_CONFIG)

$(GRUB_CONFIG):
	echo '$(subst $(NEWLINE),\n,${GRUB_CONFIG_CONTENT})' > $(GRUB_CONFIG)

clean:
	rm -rf $(BUILD_FOLDER)/**
	rm -f $(ISO_FOLDER)/boot/kernel.elf
	rm -f $(BOCHS_FOLDER)/$(BOCHS_CONFIG) $(BOCHS_FOLDER)/$(BOCHS_LOG) $(GRUB_CONFIG)
