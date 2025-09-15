#!/bin/sh
# Copyright (C) 2006-2012 OpenWrt.org
set -e -x
if [ $# -ne 5 ] && [ $# -ne 6 ]; then
    echo "SYNTAX: $0 <file> <kernel size> <kernel directory> <rootfs size> <rootfs image> [<align>]"
    exit 1
fi

OUTPUT="$1"
KERNELSIZE="$2"
KERNELDIR="$3"
ROOTFSSIZE="$4"
ROOTFSIMAGE="$5"
ALIGN="$6"
USERDATASIZE="2048"

rm -f "$OUTPUT"

head=16
sect=63

# create partition table
set $(ptgen -o "$OUTPUT" -h $head -s $sect -p "${KERNELSIZE}m" -p "${ROOTFSSIZE}m" -p "${USERDATASIZE}m" ${ALIGN:+-l $ALIGN} ${SIGNATURE:+-S 0x$SIGNATURE})

KERNELOFFSET="$(($1 / 512))"
KERNELSIZE="$2"
ROOTFSOFFSET="$(($3 / 512))"
ROOTFSSIZE="$(($4 / 512))"
USERDATAOFFSET="$(($5 / 512))"
USERDATASIZE="$(($6 / 512))"

# Using mcopy -s ... is using READDIR(3) to iterate through the directory
# entries, hence they end up in the FAT filesystem in traversal order which
# breaks reproducibility.
# Implement recursive copy with reproducible order.
dos_dircopy() {
  local entry
  local baseentry
  for entry in "$1"/* ; do
    if [ -f "$entry" ]; then
      mcopy -i "$OUTPUT.kernel" "$entry" ::"$2"
    elif [ -d "$entry" ]; then
      baseentry="$(basename "$entry")"
      mmd -i "$OUTPUT.kernel" ::"$2""$baseentry"
      dos_dircopy "$entry" "$2""$baseentry"/
    fi
  done
}

dd if=/dev/zero of="$OUTPUT" bs=512 seek="$ROOTFSOFFSET" conv=notrunc count="$ROOTFSSIZE"
dd if="$ROOTFSIMAGE" of="$OUTPUT" bs=512 seek="$ROOTFSOFFSET" conv=notrunc

[ -n "$PADDING" ] && dd if=/dev/zero of="$OUTPUT" bs=512 seek="$USERDATAOFFSET" conv=notrunc count="$USERDATASIZE"
echo "RESET000" | dd of="$OUTPUT" bs=512 seek="$USERDATAOFFSET" conv=notrunc,sync count=1

mkfs.fat --invariant -n kernel -C "$OUTPUT.kernel" -S 512 "$((KERNELSIZE / 1024))"
LC_ALL=C dos_dircopy "$KERNELDIR" /

dd if="$OUTPUT.kernel" of="$OUTPUT" bs=512 seek="$KERNELOFFSET" conv=notrunc
rm -f "$OUTPUT.kernel"
