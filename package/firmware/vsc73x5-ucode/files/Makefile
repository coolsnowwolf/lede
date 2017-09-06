#
# Copyright (C) 2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

CC:=gcc
OBJCOPY:=objcopy

all: g5_Plus1_2_31_unmanaged_Atheros_v3.bin \
     g5_Plus1_2_31_unmanaged_Atheros_v4.bin \
     g5_Plus1_2_29b_unmanaged_Atheros_v5.bin \
     g5e_Plus1_2_29a_unmanaged_Atheros_v3.bin

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

%.bin: %.o
	$(OBJCOPY) -O binary -j .data $^ $@
