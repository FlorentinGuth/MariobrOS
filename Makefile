OS_NAME = MariobrOS

# Options
GUI_LIBRARY = sdl

# Folders and paths
SRC_FOLDER   = src
BUILD_FOLDER = build
ISO_FOLDER   = iso
BOCHS_FOLDER = bochs
VPATH        = $(SRC_FOLDER) $(BUILD_FOLDER) $(BOCH_FOLDER) # Helps make find our files
$(shell mkdir -p $(BUILD_FOLDER) $(BOCHS_FOLDER))    # Ensures folders exist

# File names
LINKER = link.ld
OBJECTS = $(patsubst $(SRC_FOLDER)/%.c,%.o,$(wildcard $(SRC_FOLDER)/*.c)) \
          $(patsubst $(SRC_FOLDER)/%.s,%.o,$(wildcard $(SRC_FOLDER)/*.s))
ISO = os.iso
BOCHS_CONFIG = config.txt
BOCHS_LOG = log.txt
GRUB_CONFIG = $(ISO_FOLDER)/boot/grub/menu.lst
ELTORITO = boot/grub/stage2_eltorito

# C compiler and flags
CC =      gcc
CFLAGS =  -m32 \
          -nostdlib \
          -ffreestanding \
          -fno-builtin \
          -fno-stack-protector \
          -nostartfiles \
          -nodefaultlibs \
	      -funsigned-char \
	      -funsigned-bitfields \
          -Wall -Wextra -Werror \
          -O0 \
          -c
# Compiles in 32 bits mode, without any std, with all warnings (and treating warning as errors) \
  and with no linking (and disable optimizations)

# Linker and flags
LD =      ld
LDFLAGS = -T $(SRC_FOLDER)/$(LINKER) \
          -melf_i386

# ASM compiler and flags
AS =      nasm
ASFLAGS = -f elf32


# Configuration files
define BOCHS_CONFIG_CONTENT
megs:             32
display_library:  $(GUI_LIBRARY)
romimage:         file=/usr/share/bochs/BIOS-bochs-latest
vgaromimage:      file=/usr/share/bochs/VGABIOS-lgpl-latest
ata0-master:      type=cdrom, path=$(BUILD_FOLDER)/$(ISO), status=inserted
boot:             cdrom
log:              $(BOCHS_FOLDER)/$(BOCHS_LOG)
clock:            sync=realtime, time0=local
cpu:              count=1, ips=1000000
com1:             enabled=1, mode=file, dev=$(BOCHS_FOLDER)/com1.out
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
	bochs -q -f $(BOCHS_FOLDER)/$(BOCHS_CONFIG)


$(ISO):	kernel.elf $(GRUB_CONFIG)
    # Builds the ISO image from the ISO folder
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

kernel.elf:	$(OBJECTS)
    # Links the file and produces the .elf in the ISO folder
	$(LD) $(LDFLAGS) $(addprefix $(BUILD_FOLDER)/,$(OBJECTS)) -o $(ISO_FOLDER)/boot/kernel.elf


ECHO_CONFIG = @echo '$(subst $(NEWLINE),\n,$(1))' > $(2)

$(BOCHS_CONFIG):
	$(call ECHO_CONFIG,$(BOCHS_CONFIG_CONTENT),$(BOCHS_FOLDER)/$(BOCHS_CONFIG))

$(GRUB_CONFIG):
	$(call ECHO_CONFIG,$(GRUB_CONFIG_CONTENT),$(GRUB_CONFIG))


clean:
	rm -rf $(BUILD_FOLDER) $(BOCHS_FOLDER)
	rm -f $(ISO_FOLDER)/boot/kernel.elf
	rm -f $(GRUB_CONFIG)


# Brace yourselves, here comes the C compilation with automatic dependency generation
# For explanations (you probably don't want any, let me tell you), see:
# http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/

SRCS = (wildcard $(SRC_FOLDER)/*.c)
DEPDIR = $(BUILD_FOLDER)
$(shell mkdir -p $(DEPDIR))
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS)
POSTCOMPILE = mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

%.o:	%.c
%.o:	%.c $(DEPDIR)/%.d
	$(COMPILE.c) -o $(BUILD_FOLDER)/$@ $<
	$(POSTCOMPILE)

%.o:	%.s
%.o:	%.s
	$(AS) $(ASFLAGS) -o $(BUILD_FOLDER)/$@ $<

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS))))
