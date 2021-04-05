#!/bin/sh

set -x
[ $# -eq 5 ] || {
	echo "SYNTAX: $0 <file> <bootfs image> <rootfs image> <bootfs size> <rootfs size>"
	exit 1
}

OUTPUT="$1"
BOOTFS="$2"
ROOTFS="$3"
BOOTFSSIZE="$4"
ROOTFSSIZE="$5"

head=4
sect=63

# Create one empty partitions followed by the swap partition, then the
# boot partition with the ./boot/zImage and then the rootfs partition.
# The swap partition with type 82 is 128 MB since the DNS-313 has 64 MB of
# memory so we assign twice of that as swap.
# The boot partition must always be the third partition.
# The user should use the first (blank) partition for user data storage,
# this will typically be named /dev/sda1
set $(ptgen -o $OUTPUT -h $head -s $sect -n -t 83 -p 0 -t 82 -p 128M -t 83 -p ${BOOTFSSIZE}M -t 83 -p ${ROOTFSSIZE}M)

# Swapoffset and swapsize will be $1 and $2
BOOTOFFSET="$(($3 / 512))"
BOOTSIZE="$(($4 / 512))"
ROOTFSOFFSET="$(($5 / 512))"
ROOTFSSIZE="$(($6 / 512))"

dd bs=512 if="$BOOTFS" of="$OUTPUT" seek="$BOOTOFFSET" conv=notrunc
dd bs=512 if="$ROOTFS" of="$OUTPUT" seek="$ROOTFSOFFSET" conv=notrunc
