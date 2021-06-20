# SPDX-License-Identifier: GPL-2.0-or-later

#
# Makefile for the LZMA compressed kernel loader for BMIPS based boards
#
# Copyright (C) 2020 Álvaro Fernández Rojas <noltari@gmail.com>
# Copyright (C) 2014 Jonas Gorski <jogo@openwrt.org>
# Copyright (C) 2011 Gabor Juhos <juhosg@openwrt.org>
#
# Some parts of this file was based on the OpenWrt specific lzma-loader
# for the BCM47xx and ADM5120 based boards:
#	Copyright (C) 2004 Manuel Novoa III (mjn3@codepoet.org)
#	Copyright (C) 2005 Mineharu Takahara <mtakahar@yahoo.com>
#	Copyright (C) 2005 by Oleg I. Vdovikin <oleg@cs.msu.su>
#

LOADER_ADDR	:=
KERNEL_ADDR	:=
LZMA_TEXT_START	:= 0x80a00000
LOADER_DATA	:=

CC		:= $(CROSS_COMPILE)gcc
LD		:= $(CROSS_COMPILE)ld
OBJCOPY		:= $(CROSS_COMPILE)objcopy
OBJDUMP		:= $(CROSS_COMPILE)objdump

BIN_FLAGS	:= -O binary -R .reginfo -R .note -R .comment -R .mdebug \
		   -R .MIPS.abiflags -S

CFLAGS		= -D__KERNEL__ -Wall -Wstrict-prototypes -Wno-trigraphs -Os \
		  -fno-strict-aliasing -fno-common -fomit-frame-pointer -G 0 \
		  -mno-abicalls -fno-pic -ffunction-sections -pipe \
		  -ffreestanding -fhonour-copts \
		  -mabi=32 -march=mips32 \
		  -Wa,-32 -Wa,-march=mips32 -Wa,-mips32 -Wa,--trap
CFLAGS		+= -D_LZMA_PROB32
CFLAGS		+= -DUART_BASE=$(UART_BASE)

ASFLAGS		= $(CFLAGS) -D__ASSEMBLY__

LDFLAGS		= -static --gc-sections -no-warn-mismatch
LDFLAGS		+= -e startup -T loader.lds -Ttext $(LZMA_TEXT_START)

O_FORMAT 	= $(shell $(OBJDUMP) -i | head -2 | grep elf32)

OBJECTS		:= head.o loader.o cache.o board.o printf.o LzmaDecode.o

ifneq ($(strip $(LOADER_DATA)),)
OBJECTS		+= data.o
CFLAGS		+= -DLZMA_WRAPPER=1 -DLOADADDR=$(KERNEL_ADDR)
endif


all: loader.elf

# Don't build dependencies, this may die if $(CC) isn't gcc
dep:

install:

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o : %.S
	$(CC) $(ASFLAGS) -c -o $@ $<

data.o: $(LOADER_DATA)
	$(LD) -r -b binary --oformat $(O_FORMAT) -T lzma-data.lds -o $@ $<

loader: $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

loader.bin: loader
	$(OBJCOPY) $(BIN_FLAGS) $< $@

loader2.o: loader.bin
	$(LD) -r -b binary --oformat $(O_FORMAT) -o $@ $<

loader.elf: loader2.o
	$(LD) -e startup -T loader2.lds -Ttext $(LOADER_ADDR) -o $@ $<

mrproper: clean

clean:
	rm -f loader *.elf *.bin *.o
