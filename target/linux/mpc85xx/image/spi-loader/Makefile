# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (C) 2022 Matthias Schiffer <mschiffer@universe-factory.net>

CONFIG     ?= $(error No configuration set)

include config/$(CONFIG).mk

MKIMAGE    := mkimage
KARCH      := powerpc
CC         := $(CROSS_COMPILE)gcc
LD         := $(CROSS_COMPILE)ld
OBJCOPY    := $(CROSS_COMPILE)objcopy

PROGRAM_NAME := MPC85xx SPI loader

BIN_FLAGS  := -O binary --pad-to=$(PAD_TO)

CFLAGS     += -std=gnu17 -Os -Wall -Wstrict-prototypes \
		-fomit-frame-pointer -ffreestanding \
		-ffunction-sections -fno-pic \
		-Iinclude -include ../config/$(CONFIG).h \
		-DCONFIG_PROGRAM_NAME='"$(PROGRAM_NAME)"' \
		-DCONFIG_IMAGE_OFFSET=$(IMAGE_OFFSET)
ASFLAGS    := $(CFLAGS)

LDS        := loader.lds
LDFLAGS    := -static --gc-sections -T $(LDS) -Ttext $(TEXT_START)

OBJECTS    := head.o loader.o string.o stdio.o  drivers/serial/ns16550.o \
		drivers/spi/fsl_espi.o drivers/spi/spi-nor.o

OUTDIR     := out

all: $(OUTDIR)/uImage

-include $(OBJECTS:%.o=$(OUTDIR)/%.d)

$(OUTDIR)/%.o: %.c Makefile config/$(CONFIG).mk
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ -MD -MP $<

$(OUTDIR)/%.o: %.S Makefile config/$(CONFIG).mk
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c -o $@ -MD -MP $<

$(OUTDIR)/loader.elf: $(OBJECTS:%=$(OUTDIR)/%) $(LDS)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS:%=$(OUTDIR)/%)

$(OUTDIR)/loader.bin: $(OUTDIR)/loader.elf
	$(OBJCOPY) $(BIN_FLAGS) $< $@

$(OUTDIR)/uImage: $(OUTDIR)/loader.bin
	$(MKIMAGE) -A $(KARCH) -O linux -T kernel -C none \
		-a $(TEXT_START) -e $(TEXT_START) -n '$(PROGRAM_NAME)' -d $< $@

clean:
	rm -rf $(OUTDIR)

.PHONY: all clean
