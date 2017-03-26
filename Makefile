SRC_FOLDER = src
BUILD_FOLDER = build
ISO_FOLDER = iso
BOCHS_FOLDER = bochs

VPATH = $(SRC_FOLDER) $(BUILD_FOLDER)	# Helps make find our files

LINKER = link.ld
OBJECTS = loader.o kmain.o
ISO = os.iso
BOCHS_CONFIG = config.txt		# WARNING: modify if some paths change!
BOCHS_LOg = log.txt
ELTORITO = boot/grub/stage2_eltorito

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

LD =			ld
LDFLAGS = -T $(SRC_FOLDER)/$(LINKER) \
					-melf_i386

AS = 			nasm
ASFLAGS =	-f elf32

all:	run

run:	$(ISO)
  # Let's run it!
	bochs -f $(BOCHS_FOLDER)/$(BOCHS_CONFIG)

$(ISO):	kernel.elf
  # Builds the iso image from the iso folder
	genisoimage -R \
							-b $(ELTORITO) \
							-no-emul-boot \
							-boot-load-size 4 \
							-A MariobrOS \
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
 
clean:
	rm -rf $(BUILD_FOLDER)/**
	rm -f $(ISO_FOLDER)/boot/kernel.elf
