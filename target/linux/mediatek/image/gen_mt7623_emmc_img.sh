#!/bin/sh

OUTPUT_FILE=$1
KERNEL_FILE=$2
RECOVERY_FILE=$3
ROOTFS_FILE=$4

BS=512

#These to offsets are relative to the absolute location of the kernel on the mmc
#(0xA00), so their position in the image is -2560 blocks
RECOVERY_OFFSET=67072
ROOTFS_OFFSET=87552

dd bs="$BS" of="$OUTPUT_FILE" if="$KERNEL_FILE"
dd bs="$BS" of="$OUTPUT_FILE" if="$RECOVERY_FILE" seek="$RECOVERY_OFFSET"
dd bs="$BS" of="$OUTPUT_FILE" if="$ROOTFS_FILE" seek="$ROOTFS_OFFSET"
dd if=/dev/zero of="$OUTPUT_FILE" bs=128k count=1 oflag=append conv=notrunc
