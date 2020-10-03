#!/bin/bash
#
# Copyright © 2019 Alexey Loukianov <lx2@lexa2.ru>
# Copyright © 2020 David Woodhouse <dwmw2@infradead.org>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

# Generates a bootable SD card image for Banana Pi R2 (and probably
# other similar boards) as documented at
# http://www.fw-web.de/dokuwiki/doku.php?id=en:bpi-r2:storage
#
# The first sector must contain the SDMMC_BOOT header shown
# below, and also contains the MBR partition table in the end
# of the sector. The partition table must contain no active
# partitions.
#
# The second sector must contain the BRLYT header, and the
# special preloader image goes in sector 4; 2KiB into the image.
#
# The preloader loads U-Boot from  sector 640; 320KiB into the image.
# The location and the size (512KiB) are fixed and not read from
# the partition table. We set up a partition for it merely for
# our own convenience for upgrades, etc.
#
# The second partition is a FAT file system containing the kernel
# image and a uboot.env file, which is provided to this script as
# $4 (bootfs image). Its size is configurable with the
# CONFIG_MTK_BOOT_PARTSIZE option; by default 32MiB.
#
# The root filesystem comes next in the third partition.
#
#
#   ------------------------   Sector   Offset
#   |  MBR + SDMMC_BOOT    |     0       0x0
#   |----------------------|
#   |     BRLYT header     |     1       0x200
#   |----------------------|
#   .                      .
#   .                      .
#   |----------------------|
#   |                      |     4       0x800
#   |                      |
#   |     Preloader        |
#   .                      .
#   .                      .
#   |                      |     639
#   |----------------------|
#   |   MBR partition #1   |     640     0x50000
#   |                      |
#   |       U-Boot         |
#   .                      .
#   .                      .
#   |                      |     1663
#   |----------------------|
#   |   MBR partition #2   |
#   |                      |
#   |    FAT partition     |   ( MTK_BOOT_PARTSIZE
#   .                      .     default 32MiB )
#   .    (kernel, uEnv)    .
#   |                      |
#   |----------------------|
#   |   MBR partition #3   |
#   |                      |
#   |   Root partition     |
#   |                      |   ( TARGET_ROOTFS_PARTSIZE
#   |  (squashfs+overlay   |     default 104MiB )
#   .    or ext4, etc.)    .
#   .                      .
#   |                      |
#   ------------------------
#
# For eMMC boot, everything up to and including the preloader must be
# written to /dev/mmcblk0boot0, with the SDMMC_BOOT header changed to
# read EMMC_BOOT\0 instead.
#
# The contents of the main eMMC are identical to the SD card layout,
# with the preloader loading 512KiB of U-Boot starting at 0x50000.

usage() {
    echo "SYNTAX: $0 sd <file> <preloader image> <u-boot image> <u-boot offset> <bootfs image> <rootfs image> <bootfs size> <rootfs size>"
    echo " OR:    $0 emmc <file> <preloader image>"
    exit 1
}

set -e

PRELOADER_OFFSET=2     # 2KiB

SDMMC_BOOT="SDMMC_BOOT\x00\x00\x01\x00\x00\x00\x00\x02\x00\x00"
EMMC_BOOT="EMMC_BOOT\x00\x00\x00\x01\x00\x00\x00\x00\x02\x00\x00"
BRLYT="BRLYT\x00\x00\x00\x01\x00\x00\x00\x00\x08\x00\x00\
\x00\x08\x00\x00\x42\x42\x42\x42\x08\x00\x01\x00\x00\x08\x00\x00\
\x00\x08\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

case $1 in
 sd)
	[ $# -eq 9 ] || usage
	OUTPUT="$2"
	PRELOADER="$3"
	UBOOT="$4"
	UBOOTOFS="$5"
	BOOTFS="$6"
	ROOTFS="$7"
	BOOTFSSIZE="$8"
	ROOTFSSIZE="$9"

	head=4
	sect=63

	set $(ptgen -o $OUTPUT -h $head -s $sect -a 0 -l 1024 \
		    -t 41 -p 512k@${UBOOTOFS} \
		    -t c -p ${BOOTFSSIZE}M \
		    -t 83 -p ${ROOTFSSIZE}M )

	UBOOT_OFFSET="$(($1 / 512))"
	UBOOT_SIZE="$(($2 / 512))"
	BOOTFS_OFFSET="$(($3 / 512))"
	BOOTFS_SIZE="$(($4 / 512))"
	ROOTFS_OFFSET="$(($5 / 512))"
	ROOTFS_SIZE="$(($6 / 512))"

	echo -en "${SDMMC_BOOT}" | dd bs=1 of="${OUTPUT}" seek=0   conv=notrunc
	echo -en "${BRLYT}"      | dd bs=1 of="${OUTPUT}" seek=512 conv=notrunc

	# For eMMC-only boards like U7623 the preloader doesn't need to be included in the
	# main image as it's only ever needed in the eMMC boot partition.
	if [ -r ${PRELOADER} ]; then
	    dd bs=1024 if="${PRELOADER}" of="${OUTPUT}" seek="${PRELOADER_OFFSET}" conv=notrunc
	fi
	dd bs=512  if="${UBOOT}"     of="${OUTPUT}" seek="${UBOOT_OFFSET}"     conv=notrunc
	dd bs=512  if="${BOOTFS}"    of="${OUTPUT}" seek="${BOOTFS_OFFSET}"    conv=notrunc
	dd bs=512  if="${ROOTFS}"    of="${OUTPUT}" seek="${ROOTFS_OFFSET}"    conv=notrunc
	dd bs=128k if=/dev/zero      of="${OUTPUT}" count=1    oflag=append    conv=notrunc
	;;
 emmc)
	[ $# -eq 3 ] || usage
	OUTPUT="$2"
	PRELOADER="$3"

	echo -en "${EMMC_BOOT}"  | dd bs=1 of="${OUTPUT}" seek=0
	echo -en "${BRLYT}"      | dd bs=1 of="${OUTPUT}" seek=512 conv=notrunc

	dd bs=1024 if="${PRELOADER}" of="${OUTPUT}" seek="${PRELOADER_OFFSET}" conv=notrunc
	;;
 *)
	usage
	;;
esac
