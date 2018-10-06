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

TEXT_START	:= 0x80001000
BZ_TEXT_START	:= 0x80600000
BZ_STACK_START	:= 0x80700000

OBJCOPY		:= $(CROSS_COMPILE)objcopy -O binary -R .reginfo -R .note -R .comment -R .mdebug -S

CFLAGS		= -D__KERNEL__ -Wall -Wstrict-prototypes -Wno-trigraphs -Os \
		  -fno-strict-aliasing -fno-common -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic \
		  -ffunction-sections -pipe -mlong-calls -fno-common \
		  -mabi=32 -march=mips32 -Wa,-32 -Wa,-march=mips32 -Wa,-mips32 -Wa,--trap
CFLAGS		+= -DLOADADDR=$(TEXT_START) -D_LZMA_IN_CB

ASFLAGS		= $(CFLAGS) -D__ASSEMBLY__ -DBZ_TEXT_START=$(BZ_TEXT_START) -DBZ_STACK_START=$(BZ_STACK_START)

SEDFLAGS	:= s/BZ_TEXT_START/$(BZ_TEXT_START)/;s/BZ_STACK_START/$(BZ_STACK_START)/;s/TEXT_START/$(TEXT_START)/

OBJECTS		:= head.o data.o

all: loader.gz loader.elf

# Don't build dependencies, this may die if $(CC) isn't gcc
dep:

install:

loader.gz: loader
	gzip -nc9 $< > $@

loader.elf: loader.o
	cp $< $@

loader: loader.o
	$(OBJCOPY) $< $@

loader.o: loader.lds $(OBJECTS)
	$(LD) -static --gc-sections -no-warn-mismatch -T loader.lds -o $@ $(OBJECTS)

loader.lds: loader.lds.in Makefile
	@sed "$(SEDFLAGS)" < $< > $@

data.o: data.lds decompress.image
	$(LD) -no-warn-mismatch -T data.lds -r -o $@ -b binary decompress.image -b elf32-tradlittlemips

data.lds:
	@echo "SECTIONS { .data : { code_start = .; *(.data) code_stop = .; }}" > $@

decompress.image: decompress
	$(OBJCOPY) $< $@

decompress: decompress.lds decompress.o LzmaDecode.o
	$(LD) -static --gc-sections -no-warn-mismatch -T decompress.lds -o $@ decompress.o LzmaDecode.o

decompress.lds: decompress.lds.in Makefile
	@sed "$(SEDFLAGS)" < $< > $@

mrproper: clean

clean:
	rm -f loader.gz loader decompress *.lds *.o *.image
