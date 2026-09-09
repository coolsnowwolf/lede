#!/bin/sh
# Copyright (C) 2006-2012 OpenWrt.org
set -e -x
if [ $# -lt 5 ] || [ $# -gt 7 ]; then
    echo "SYNTAX: $0 <file> <kernel size> <kernel directory> <rootfs size> <rootfs image> [<align>] [<logical-sector-size>]"
    exit 1
fi

OUTPUT="$1"
KERNELSIZE="$2"
KERNELDIR="$3"
ROOTFSSIZE="$4"
ROOTFSIMAGE="$5"
ALIGN="$6"
LOGICAL_SECTOR_SIZE="${7:-512}"

if [ "$LOGICAL_SECTOR_SIZE" -lt 512 ] || [ $((LOGICAL_SECTOR_SIZE % 512)) -ne 0 ]; then
    echo "Invalid logical sector size: $LOGICAL_SECTOR_SIZE"
    exit 1
fi

rm -f "$OUTPUT"

head=16
sect=63
KERNELPART="${KERNELSIZE}m"
[ -n "$KERNEL_OFFSET" ] && KERNELPART="${KERNELPART}@${KERNEL_OFFSET}k"

# create partition table
if [ -n "$GUID" ] && [ -n "$PURE_GPT" ]; then
    set $(ptgen -o "$OUTPUT" -h $head -s $sect -g -D -p "$KERNELPART" -p "${ROOTFSSIZE}m" ${ALIGN:+-l $ALIGN} -G "$GUID")
    GPT_DISK_SIZE=$((($3 + $4 + 1023) / 1024 + ${GPT_PADDING_KB:-1024}))
    set $(ptgen -o "$OUTPUT" -h $head -s $sect -g -D -d "$GPT_DISK_SIZE" -p "$KERNELPART" -p "${ROOTFSSIZE}m" ${ALIGN:+-l $ALIGN} -G "$GUID")
else
    set $(ptgen -o "$OUTPUT" -h $head -s $sect ${GUID:+-g} -p "$KERNELPART" -p "${ROOTFSSIZE}m" ${ALIGN:+-l $ALIGN} ${SIGNATURE:+-S 0x$SIGNATURE} ${GUID:+-G $GUID})
fi

KERNELOFFSET="$(($1 / 512))"
KERNELSIZE="$2"
ROOTFSOFFSET="$(($3 / 512))"
ROOTFSSIZE="$(($4 / 512))"
ROOTFSOFFSET_ALT="$((ROOTFSOFFSET * (LOGICAL_SECTOR_SIZE / 512)))"

[ -n "$PADDING" ] && dd if=/dev/zero of="$OUTPUT" bs=512 seek="$ROOTFSOFFSET" conv=notrunc count="$ROOTFSSIZE"
dd if="$ROOTFSIMAGE" of="$OUTPUT" bs=512 seek="$ROOTFSOFFSET" conv=notrunc

if [ "$ROOTFSOFFSET_ALT" -ne "$ROOTFSOFFSET" ]; then
    [ -n "$PADDING" ] && dd if=/dev/zero of="$OUTPUT" bs=512 seek="$ROOTFSOFFSET_ALT" conv=notrunc count="$ROOTFSSIZE"
    dd if="$ROOTFSIMAGE" of="$OUTPUT" bs=512 seek="$ROOTFSOFFSET_ALT" conv=notrunc
fi

if [ -n "$KERNEL_IMAGE" ]; then
    KERNEL_IMAGE_SIZE=$(stat -c%s "$KERNEL_IMAGE")
    if [ "$KERNEL_IMAGE_SIZE" -gt "$KERNELSIZE" ]; then
        echo "Kernel image exceeds configured kernel partition size"
        exit 1
    fi
    dd if="$KERNEL_IMAGE" of="$OUTPUT" bs=512 seek="$KERNELOFFSET" conv=notrunc
elif [ -n "$GUID" ]; then
    [ -n "$PADDING" ] && dd if=/dev/zero of="$OUTPUT" bs=512 seek="$((ROOTFSOFFSET + ROOTFSSIZE))" conv=notrunc count="$sect"
    mkfs.fat -n kernel -C "$OUTPUT.kernel" -S 512 "$((KERNELSIZE / 1024))"
    mcopy -s -i "$OUTPUT.kernel" "$KERNELDIR"/* ::/
else
    make_ext4fs -J -L kernel -l "$KERNELSIZE" "$OUTPUT.kernel" "$KERNELDIR"
fi
if [ -z "$KERNEL_IMAGE" ]; then
    dd if="$OUTPUT.kernel" of="$OUTPUT" bs=512 seek="$KERNELOFFSET" conv=notrunc
    rm -f "$OUTPUT.kernel"
fi
