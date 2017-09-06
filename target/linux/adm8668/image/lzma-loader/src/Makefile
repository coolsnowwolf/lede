#
# Copyright (C) 2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
RAMSTART	= 0x80000000
RAMSIZE		= 0x00800000		# 8MB
LOADADDR	= 0x80400000		# RAM start + 4M
KERNEL_ENTRY	= 0x80002000

CROSS_COMPILE = mipsel-openwrt-linux-

OBJCOPY:= $(CROSS_COMPILE)objcopy -O binary -R .reginfo -R .note -R .comment -R .mdebug -S
CFLAGS := -I./include -fno-builtin -Os -G 0 -ffunction-sections -mno-abicalls -fno-pic -mabi=32 -march=mips32 -Wa,-32 -Wa,-march=mips32 -Wa,-mips32 -Wa,--trap -Wall -DRAMSTART=${RAMSTART} -DRAMSIZE=${RAMSIZE} -DKERNEL_ENTRY=${KERNEL_ENTRY}

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
	sed -e 's,@LOADADDR@,$(LOADADDR),g' $< >$@

kernel.o: vmlinux.lzma lzma.lds
	$(LD) -r -b binary --oformat $(O_FORMAT) -o $@ $<

lzma.bin: lzma.elf
	$(OBJCOPY) $< $@

lzma.elf: decompress.o stubs.o LzmaDecode.o kernel.o lzma.lds
	$(LD) -T lzma.lds -o $@ $^
#-s ^

clean:
	rm -f *.o lzma.elf lzma.bin *.tmp *.lds
