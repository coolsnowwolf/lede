#!/bin/sh
#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

set -x
[ $# -eq 4 ] || {
    echo "SYNTAX: $0 <file> <u-boot.sb image> <rootfs image> <rootfs size>"
    exit 1
}

OUTPUT="$1"
UBOOT="$2"
ROOTFS="$3"
ROOTFSSIZE="$4"

head=4
sect=63

# set the Boot stream partition size to 1M
set $(ptgen -o $OUTPUT -h $head -s $sect -l 1024 -t 53 -p 1M -t 83 -p ${ROOTFSSIZE}M -t 83 -p ${ROOTFSSIZE}M)

ROOTFS1OFFSET="$(($3 / 512))"
ROOTFS1SIZE="$(($4 / 512))"
ROOTFS2OFFSET="$(($5 / 512))"
ROOTFS2SIZE="$(($6 / 512))"

dd bs=512 if="$ROOTFS" of="$OUTPUT" seek="$ROOTFS1OFFSET" conv=notrunc
dd bs=512 if="$ROOTFS" of="$OUTPUT" seek="$ROOTFS2OFFSET" conv=notrunc
sdimage -d "$OUTPUT" -f "$UBOOT"
