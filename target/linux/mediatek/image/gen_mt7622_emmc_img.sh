#!/bin/sh

OUTPUT_FILE=$1
KERNEL_FILE=$2
RECOVERY_FILE=$3
ROOTFS_FILE=$4

BS=512

#RECOVERY_OFFSET= kernel size / BS = 20M / 512 (blocks)
RECOVERY_OFFSET=40960

#ROOTFS_OFFSET = RECOVERY_OFFSET + (RECOVERY_SIZE / BS)
#	       = 40960 + (10M  / 512) (blocks)
ROOTFS_OFFSET=61440
dd bs="$BS" of="$OUTPUT_FILE" if="$KERNEL_FILE"
dd bs="$BS" of="$OUTPUT_FILE" if="$RECOVERY_FILE" seek="$RECOVERY_OFFSET"
dd bs="$BS" of="$OUTPUT_FILE" if="$ROOTFS_FILE" seek="$ROOTFS_OFFSET"
dd if=/dev/zero of="$OUTPUT_FILE" bs=128k count=1 oflag=append conv=notrunc
