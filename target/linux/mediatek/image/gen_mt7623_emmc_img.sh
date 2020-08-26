#!/bin/sh
#
# This script is used only to create the "legacy" image for UniElec U7623,
# suitable for upgrading from the vendor OpenWrt or from OpenWrt 19.07.
#
OUTPUT_FILE=$1
KERNEL_FILE=$2
RECOVERY_FILE=$3
ROOTFS_FILE=$4

BS=512

# These two offsets are relative to the absolute location of the kernel on the mmc
# (0xA00), so their position in the image is -2560 blocks
#
# They must be kept in sync with the default command line for U7623.
# blkdevparts=mmcblk0:3M@6M(recovery),256M@9M(root)"
#
# For upgrading from the vendor firmware, the total image must fit in its
# MTD 'firmware' partition, which is typically 36MiB.

# In the legacy image, the uImage starts at block 0xa00. Allow 4864 KiB (9728) blocks.
RECOVERY_OFFSET=9728
# The recovery image is 3MiB, or 6144 blocks.
ROOTFS_OFFSET=15872

dd bs="$BS" of="$OUTPUT_FILE" if="$KERNEL_FILE"
dd bs="$BS" of="$OUTPUT_FILE" if="$RECOVERY_FILE" seek="$RECOVERY_OFFSET"
dd bs="$BS" of="$OUTPUT_FILE" if="$ROOTFS_FILE" seek="$ROOTFS_OFFSET"
dd if=/dev/zero of="$OUTPUT_FILE" bs=128k count=1 oflag=append conv=notrunc
