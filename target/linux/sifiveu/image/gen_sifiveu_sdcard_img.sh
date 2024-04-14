#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2022 OpenWrt.org
#

set -ex
[ $# -eq 7 ] || {
    echo "SYNTAX: $0 <file> <bootfs image> <rootfs image> <bootfs size> <rootfs size> <u-boot ITB image> <u-boot SPL>"
    exit 1
}

OUTPUT="$1"
BOOTFS="$2"
ROOTFS="$3"
BOOTFSSIZE="$4"
ROOTFSSIZE="$5"
UBOOT="$6"
UBOOT_SPL="$7"

set $(ptgen -o $OUTPUT -v -g -T sifiveu_spl -N loader1 -p 1024 -T sifiveu_uboot -N loader2 -p 4096 -t ef -N boot -p ${BOOTFSSIZE}M -N rootfs -p ${ROOTFSSIZE}M)

ROOTFSOFFSET=$(($7 / 512))

dd bs=512 if="$UBOOT_SPL" of="$OUTPUT" seek=34 conv=notrunc
dd bs=512 if="$UBOOT" of="$OUTPUT" seek=2082 conv=notrunc
dd bs=512 if="$BOOTFS" of="$OUTPUT" seek=10274 conv=notrunc
dd bs=512 if="$ROOTFS" of="$OUTPUT" seek=${ROOTFSOFFSET} conv=notrunc
