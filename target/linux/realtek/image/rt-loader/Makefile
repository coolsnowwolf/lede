# rt-loader make file
# (c) 2025 Markus Stockhausen
#
# This is the make file for the rt-loader (aka runtime or realtek loader). It tries to
# avoid copying files around where possible. Therefore it is controlled by the following
# input parameters
#
# KERNEL_IMG_IN:	The filename of an LZMA compressed kernel. If given the loader
#			and the kernel will be concatenated (piggy back loading).
# FLASH_ADDR:		The kernel address in the ROM. If given, the loader will be
#			created standalone and search a LZMA compressed uImage on the
#			target device starting from this address.
# KERNEL_ADDR:		The to-be-used kernel load address to which rt-loader will load
# 			the kernel and execute it. If not given, rt-loader will use its
# 			initial run address for this (which is the uImage load address
# 			when using U-boot images) For non-uImage, this uses an arbitrary
# 			load address which is independent of rt-loader's initial run
# 			address.
# KERNEL_IMG_OUT:	The filename of the kernel image with the rt-loader prepended.
#			If not given it will be created as image.bin into the BUILD_DIR.
# BUILD_DIR:		The temporary build dir. If not given it will be set to "build".
#
# To add it into the OpenWrt toolchain just create the following new build commands
#
# define Build/rt-compress
#   $(STAGING_DIR_HOST)/bin/xz --format=lzma -9 --stdout "$@" > "$@.new"
#   mv "$@.new" "$@"
# endef
#
# define Build/rt-loader-piggy-back
#   $(MAKE) all clean -C rt-loader CROSS_COMPILE="$(TARGET_CROSS)" \
#	    KERNEL_IMG_IN="$@" KERNEL_IMG_OUT="$@.new" BUILD_DIR="$@.build"
#   mv "$@.new" "$@"
# endef
#
# define Build/rt-loader-non-uImage
#   $(MAKE) all clean -C rt-loader CROSS_COMPILE="$(TARGET_CROSS)" \
#           KERNEL_ADDR="$(KERNEL_LOADADDR)" KERNEL_IMG_IN="$@" \
#           KERNEL_IMG_OUT="$@.new" BUILD_DIR="$@.build"
#   mv "$@.new" "$@"
#
# define Build/rt-loader-standalone
#   $(MAKE) all clean -C rt-loader CROSS_COMPILE="$(TARGET_CROSS)" \
#	    FLASH_ADDR=$(FLASH_ADDR) KERNEL_IMG_OUT="$@.new" BUILD_DIR="$@.build"
#   mv "$@.new" "$@"
# endef
#
# Finally use them in new kernel build recipes. E.g. as all-in-one self extracting uImage.
#
# define Device/uimage-rt-loader
#   KERNEL/rt-loader := kernel-bin | append-dtb | rt-compress | rt-loader-piggy-back
#   KERNEL := $$(KERNEL/rt-loader) | uImage none
#   KERNEL_INITRAMFS := $$(KERNEL/rt-loader) | uImage none
# endef
#
# Or as direct startable standlone version plus a kernel uImage
#
# define Device/rt-loader-uimage
#   FLASH_ADDR := 0xb4000000
#   KERNEL/rt-loader := rt-loader-standalone
#   KERNEL := kernel-bin | append-dtb | rt-compress | uImage lzma
# endef
#

FLASH_ADDR_NONE	:= 0x0
FLASH_ADDR	?= $(FLASH_ADDR_NONE)

KERNEL_ADDR_NONE := 0x0
KERNEL_ADDR 	?= $(KERNEL_ADDR_NONE)

CC		:= $(CROSS_COMPILE)gcc
LD		:= $(CROSS_COMPILE)ld
OBJCOPY		:= $(CROSS_COMPILE)objcopy
OBJDUMP		:= $(CROSS_COMPILE)objdump

CFLAGS		= -fpic -mabicalls -O3 -fno-builtin-printf -Iinclude
CFLAGS		+= -DFLASH_ADDR=$(FLASH_ADDR)
CFLAGS		+= -DKERNEL_ADDR=$(KERNEL_ADDR)

ASFLAGS		= -fpic -msoft-float -Iinclude

LDFLAGS		= -static -nostdlib -T linker/linker.ld --no-warn-mismatch

O_FORMAT	= $(shell $(OBJDUMP) -i | head -2 | grep elf32)

SOURCES		= src/startup.S src/main.c src/board.c src/memory.c src/unlzma.c

BUILD_DIR	?= build

IMAGE_OBJ	:= $(BUILD_DIR)/image.o
IMAGE_ELF	:= $(BUILD_DIR)/image.elf

KERNEL_IMG_NONE	:= $(BUILD_DIR)/empty_kernel.bin
KERNEL_IMG_IN	?= $(KERNEL_IMG_NONE)
KERNEL_IMG_OUT	?= $(BUILD_DIR)/image.bin

OBJECTS_C	= $(filter %.c,$(SOURCES))
OBJECTS_S	= $(filter %.S,$(SOURCES))

OBJECTS		:= $(OBJECTS_S:.S=.o) $(OBJECTS_C:.c=.o)
OBJECTS		:= $(patsubst %.o, $(BUILD_DIR)/%.o, $(OBJECTS)) $(IMAGE_OBJ)

ifneq ($(MAKECMDGOALS),clean)
  ifeq ($(KERNEL_IMG_IN)$(FLASH_ADDR),$(KERNEL_IMG_NONE)$(FLASH_ADDR_NONE))
    $(error Set either KERNEL_IMG_IN or FLASH_ADDR)
  else ifneq ($(FLASH_ADDR),$(FLASH_ADDR_NONE))
    $(info Create standalone rt-loader, loading uimage from address $(FLASH_ADDR))
  else
    $(info Create piggy backed rt-loader, loading appended kernel binary "$(KERNEL_IMG_IN)")
  endif
endif

all: $(KERNEL_IMG_OUT)

install:

$(BUILD_DIR)/%.o : %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o : %.S
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c -o $@ $<

$(IMAGE_OBJ): $(KERNEL_IMG_IN)
	$(OBJCOPY) -I binary -O $(O_FORMAT) --rename-section .data=.kernel $< $@

$(IMAGE_ELF): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

$(KERNEL_IMG_OUT): $(IMAGE_ELF)
	$(OBJCOPY) -O binary $< $@

$(KERNEL_IMG_IN):
	@mkdir -p $(dir $@)
	@echo "DUMMY-KERNEL-IMAGE" > $@

clean:
	rm -rf $(BUILD_DIR)/

