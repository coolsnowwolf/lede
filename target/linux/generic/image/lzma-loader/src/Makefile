# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
RAMSTART = 0x80000000
RAMSIZE = 0x00100000		# 1MB
LOADADDR = 0x80400000		# RAM start + 4M
KERNEL_ENTRY = 0x80001000
IMAGE_COPY:=0

CROSS_COMPILE = mips-linux-

OBJCOPY:= $(CROSS_COMPILE)objcopy -O binary -R .reginfo -R .note -R .comment -R .mdebug -S
CFLAGS := -fno-builtin -Os -G 0 -ffunction-sections -mno-abicalls -fno-pic -mabi=32 -march=mips32 -Wa,-32 -Wa,-march=mips32 -Wa,-mips32 -Wa,--trap -Wall -DRAMSTART=${RAMSTART} -DRAMSIZE=${RAMSIZE} -DKERNEL_ENTRY=${KERNEL_ENTRY} -D_LZMA_IN_CB
ifeq ($(IMAGE_COPY),1)
CFLAGS += -DLOADADDR=${LOADADDR} -DIMAGE_COPY=1
endif

.S.s:
	$(CPP) $(CFLAGS) $< -o $*.s
.S.o:
	$(CC) $(CFLAGS) -c $< -o $*.o
.c.o:
	$(CC) $(CFLAGS) -c $< -o $*.o

CC =       $(CROSS_COMPILE)gcc
LD =       $(CROSS_COMPILE)ld
OBJDUMP =  $(CROSS_COMPILE)objdump

O_FORMAT = $(shell $(OBJDUMP) -i | head -2 | grep elf32)

# Drop some uninteresting sections in the kernel.
# This is only relevant for ELF kernels but doesn't hurt a.out
drop-sections   = .reginfo .mdebug .comment
strip-flags     = $(addprefix --remove-section=,$(drop-sections))

all : lzma.elf lzma.bin

lzma.lds: lzma.lds.in
	sed -e 's,@LOADADDR@,$(LOADADDR),g' -e 's,@ENTRY@,_start,g' $< >$@

kernel.o: vmlinux.lzma lzma.lds
	$(LD) -r -b binary --oformat $(O_FORMAT) -o $@ $<

lzma.bin: lzma.elf
	$(OBJCOPY) $< $@

ifeq ($(IMAGE_COPY),1)
LOADER_ENTRY ?= $(KERNEL_ENTRY)
lzma.o: decompress.o LzmaDecode.o kernel.o
	sed -e 's,@LOADADDR@,$(LOADADDR),g' -e 's,@ENTRY@,entry,g' lzma.lds.in >lzma-stage2.lds
	$(LD) -static --no-warn-mismatch -e entry -Tlzma-stage2.lds -o temp-$@ $^
	$(OBJCOPY) temp-$@ lzma.tmp
	@echo "SECTIONS { .data : { code_start = .; *(.data) code_stop = .; }}" > lzma-data.lds
	$(LD) -no-warn-mismatch -T lzma-data.lds -r -o $@ -b binary lzma.tmp --oformat $(O_FORMAT)
	
lzma.elf: start.o lzma.o
	sed -e 's,@LOADADDR@,$(LOADER_ENTRY),g' lzma-copy.lds.in >lzma-copy.lds
	$(LD) -s -Tlzma-copy.lds -o $@ $^
else
lzma.elf: start.o decompress.o LzmaDecode.o kernel.o
	$(LD) -s -Tlzma.lds -o $@ $^
endif

clean:
	rm -f *.o lzma.elf lzma.bin *.tmp *.lds
