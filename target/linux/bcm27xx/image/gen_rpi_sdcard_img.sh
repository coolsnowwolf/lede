#!/bin/sh

set -e -x

if [ $# -ne 5 ]; then
    echo "SYNTAX: $0 <file> <bootfs image> <rootfs image> <bootfs size> <rootfs size>"
    exit 1
fi

OUTPUT="$1"
BOOTFS="$2"
ROOTFS="$3"
BOOTFSSIZE="$4"
ROOTFSSIZE="$5"

align=4096
head=4
kernel_type=c
rootfs_type=83
sect=63

set $(ptgen -o $OUTPUT -h $head -s $sect -l $align -t $kernel_type -p ${BOOTFSSIZE}M -t $rootfs_type -p ${ROOTFSSIZE}M ${SIGNATURE:+-S 0x$SIGNATURE})

BOOTOFFSET="$(($1 / 512))"
ROOTFSOFFSET="$(($3 / 512))"

# In most cases, the rootfs image size is smaller than the total space
# available in the rootfs. OpenWrt will use the remaining space for storing
# the fs backing the overlayfs which holds configuration changes etc.
# To avoid that OpenWrt incorrectly detects a filesystem in the case
# there is remaining data from a previous installation, we need to clear
# the first N bytes directly after the rootfs image, to make sure
# that any initial bytes that are or look like fs superblocks are cleared
# out.
ROOTFSSIZE="$(($4 / 512))"
ROOTFSIMGSIZE="$((($(wc -c < $ROOTFS) + 511) / 512))"
ROOTFSPADDINGSIZE="$(($ROOTFSSIZE - $ROOTFSIMGSIZE))"
ROOTFSPADDINGOFFSET="$(($ROOTFSOFFSET + $ROOTFSIMGSIZE))"

# Reduce padding to max 1MB, which is enough to clear out any superblocks
# of previous file-systems or any data that might look like fs superblocks.
if [ "$ROOTFSPADDINGSIZE" -gt 2048 ]; then
  ROOTFSPADDINGSIZE="2048"
fi

# Write the bootfs.
dd bs=512 if="$BOOTFS" of="$OUTPUT" seek="$BOOTOFFSET" conv=notrunc

# Write the rootfs.
# The `sync` is important, as in case $ROOTFS is not a multiple of bs, it will
# assure that remaining bytes are set to 0x00.
dd bs=512 if="$ROOTFS" of="$OUTPUT" seek="$ROOTFSOFFSET" conv=notrunc,sync

# Add padding after rootfs.
if [ "$ROOTFSPADDINGSIZE" -gt 0 ]; then
  dd bs=512 if=/dev/zero of="$OUTPUT" seek="$ROOTFSPADDINGOFFSET" count="$ROOTFSPADDINGSIZE" conv=notrunc
fi
