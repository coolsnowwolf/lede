#!/usr/bin/env bash
# Copyright (C) 2006-2012 OpenWrt.org
set -e -x
[ $# == 5 -o $# == 6 ] || {
    echo "SYNTAX: $0 <file> <kernel size> <kernel directory> <rootfs size> <rootfs image> [<align>]"
    exit 1
}

OUTPUT="$1"
KERNELSIZE="$2"
KERNELDIR="$3"
ROOTFSSIZE="$4"
ROOTFSIMAGE="$5"
ALIGN="$6"

rm -f "$OUTPUT"

head=16
sect=63
cyl=$(( (KERNELSIZE + ROOTFSSIZE) * 1024 * 1024 / (head * sect * 512)))

# create partition table
set $(ptgen -o "$OUTPUT" -h $head -s $sect ${GUID:+-g} -p ${KERNELSIZE}m -p ${ROOTFSSIZE}m ${ALIGN:+-l $ALIGN} ${SIGNATURE:+-S 0x$SIGNATURE} ${GUID:+-G $GUID})

KERNELOFFSET="$(($1 / 512))"
KERNELSIZE="$2"
ROOTFSOFFSET="$(($3 / 512))"
ROOTFSSIZE="$(($4 / 512))"

[ -n "$PADDING" ] && dd if=/dev/zero of="$OUTPUT" bs=512 seek="$ROOTFSOFFSET" conv=notrunc count="$ROOTFSSIZE"
dd if="$ROOTFSIMAGE" of="$OUTPUT" bs=512 seek="$ROOTFSOFFSET" conv=notrunc

if [ -n "$GUID" ]; then
    [ -n "$PADDING" ] && dd if=/dev/zero of="$OUTPUT" bs=512 seek="$(($ROOTFSOFFSET + $ROOTFSSIZE))" conv=notrunc count="$sect"
    mkfs.fat -n kernel -C "$OUTPUT.kernel" -S 512 "$(($KERNELSIZE / 1024))"
    mcopy -s -i "$OUTPUT.kernel" "$KERNELDIR"/* ::/
else
    make_ext4fs -J -L kernel -l "$KERNELSIZE" "$OUTPUT.kernel" "$KERNELDIR"
fi
dd if="$OUTPUT.kernel" of="$OUTPUT" bs=512 seek="$KERNELOFFSET" conv=notrunc
rm -f "$OUTPUT.kernel"
