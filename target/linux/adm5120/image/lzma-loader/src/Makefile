#
# Makefile for Broadcom BCM947XX boards
#
# Copyright 2001-2003, Broadcom Corporation
# All Rights Reserved.
#
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# Copyright 2004  Manuel Novoa III <mjn3@codepoet.org>
#   Modified to support bzip'd kernels.
#   Of course, it would be better to integrate bunzip capability into CFE.
#
# Copyright 2005  Oleg I. Vdovikin <oleg@cs.msu.su>
#   Cleaned up, modified for lzma support, removed from kernel
#
# Copyright 2007 Gabor Juhos <juhosg at openwrt.org>
#   Modified to support user defined entry point address.
#   Added support for make targets with different names
#

LOADADDR	:= 0x80001000
LZMA_TEXT_START	:= 0x80500000
LZMA_STARTUP_ORG:= 0
LOADER_DATA	:=
CONFIG_PASS_KARGS :=
CONFIG_BOARD	:=

CC		:= $(CROSS_COMPILE)gcc
LD		:= $(CROSS_COMPILE)ld
OBJCOPY		:= $(CROSS_COMPILE)objcopy
OBJDUMP		:= $(CROSS_COMPILE)objdump

BIN_FLAGS	:= -O binary -R .reginfo -R .note -R .comment -R .mdebug -S

CFLAGS		= -D__KERNEL__ -Wall -Wstrict-prototypes -Wno-trigraphs -Os \
		  -fno-strict-aliasing -fno-common -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic \
		  -ffunction-sections -pipe -mlong-calls -fno-common -ffreestanding \
		  -fhonour-copts \
		  -mabi=32 -march=mips32 -Wa,-32 -Wa,-march=mips32 -Wa,-mips32 -Wa,--trap
CFLAGS		+= -DLOADADDR=$(LOADADDR) -D_LZMA_PROB32

ASFLAGS		= $(CFLAGS) -D__ASSEMBLY__ -DLZMA_STARTUP_ORG=$(LZMA_STARTUP_ORG)

LDFLAGS		= -static --gc-sections -no-warn-mismatch
LDFLAGS		+= -e startup -T loader.lds -Ttext $(LZMA_TEXT_START)

O_FORMAT 	= $(shell $(OBJDUMP) -i | head -2 | grep elf32)

OBJECTS		:= head.o decompress.o board.o printf.o LzmaDecode.o

ifneq ($(strip $(LOADER_DATA)),)
OBJECTS		+= data.o
CFLAGS		+= -DLZMA_WRAPPER=1
else
CFLAGS		+= -D_LZMA_IN_CB
endif

ifneq ($(strip $(CONFIG_PASS_KARGS)),)
CFLAGS		+= -DCONFIG_PASS_KARGS
endif

BOARD_DEF := $(strip $(CONFIG_BOARD))
BOARD_DEF := $(shell echo $(BOARD_DEF) | tr a-z A-Z | tr -d -)
ifneq ($(BOARD_DEF),)
CFLAGS		+= -DCONFIG_BOARD_$(BOARD_DEF)
endif

all: loader.bin

# Don't build dependencies, this may die if $(CC) isn't gcc
dep:

install:

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o : %.S
	$(CC) $(ASFLAGS) -c -o $@ $<

data.o: $(LOADER_DATA)
	$(LD) -r -b binary --oformat $(O_FORMAT) -T lzma-data.lds -o $@ $<

loader.bin: loader.elf
	$(OBJCOPY) $(BIN_FLAGS) $< $@

loader.elf: $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

mrproper: clean

clean:
	rm -f *.elf *.bin *.o



