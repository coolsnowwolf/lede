#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2015 OpenWrt.org

set -x
[ $# -eq 6 ] || {
    echo "SYNTAX: $0 <file> <u-boot.sb image> <bootfs image> <rootfs image> <bootfs size> <rootfs size>"
    exit 1
}

OUTPUT="$1"
UBOOT="$2"
BOOTFS="$3"
ROOTFS="$4"
BOOTFSSIZE="$5"
ROOTFSSIZE="$6"

head=4
sect=63

# Set the u-boot storage to 2M
set $(ptgen -o $OUTPUT -h $head -s $sect -l 1024 -t 53 -p 2M -t c -p ${BOOTFSSIZE}M -t 83 -p ${ROOTFSSIZE}M)

UBOOTOFFSET="$(($1 / 512))"
UBOOTSIZE="$(($2 / 512))"
BOOTOFFSET="$(($3 / 512))"
BOOTSIZE="$(($4 / 512))"
ROOTFSOFFSET="$(($5 / 512))"
ROOTFSSIZE="$(($6 / 512))"

dd bs=512 if="$BOOTFS" of="$OUTPUT" seek="$BOOTOFFSET" conv=notrunc
dd bs=512 if="$ROOTFS" of="$OUTPUT" seek="$ROOTFSOFFSET" conv=notrunc
sdimage -d "$OUTPUT" -f "$UBOOT"
