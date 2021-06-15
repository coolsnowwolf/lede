#!/usr/bin/env bash
set -x
[ $# == 8 -o $# == 9 ] || {
    echo "SYNTAX: $0 <file> <kernel size> <kernel directory> <efi size> <efi image> <efigrubmodule size> <rootfs size> <rootfs image> [<align>]"
    exit 1
}

OUTPUT="$1"
KERNELSIZE="$2"
KERNELDIR="$3"
EFISIZE="$4"
EFIIMAGE="$5"
EFIGRUBSIZE="$6"
ROOTFSSIZE="$7"
ROOTFSIMAGE="$8"
ALIGN="$9"

rm -f "$OUTPUT"

head=16
sect=63
cyl=$(( ($KERNELSIZE + $EFISIZE + $EFIGRUBSIZE + $ROOTFSSIZE) * 1024 * 1024 / ($head * $sect * 512) ))

# create partition table
set `ptgen -o "$OUTPUT" -h $head -s $sect -p ${KERNELSIZE}m -p ${EFISIZE}m -p ${EFIGRUBSIZE}m -p ${ROOTFSSIZE}m ${ALIGN:+-l $ALIGN} ${SIGNATURE:+-S 0x$SIGNATURE}`

KERNELOFFSET="$(($1 / 512))"
KERNELSIZE="$2"
EFIOFFSET="$(($3 / 512))"
EFISIZE="$(($4 / 512))"
EFIGRUBOFFSET="$(($5 / 512))"
EFIGRUBSIZE="$(($6 / 512))"
ROOTFSOFFSET="$(($7 / 512))"
ROOTFSSIZE="$(($8 / 512))"

dd if=/dev/zero of="$OUTPUT" bs=512 seek="$ROOTFSOFFSET" conv=notrunc count="$ROOTFSSIZE"
dd if="$ROOTFSIMAGE" of="$OUTPUT" bs=512 seek="$ROOTFSOFFSET" conv=notrunc
dd if="$EFIIMAGE" of="$OUTPUT" bs=512 seek="$EFIOFFSET" conv=notrunc

[ -n "$NOGRUB" ] && exit 0

make_ext4fs -J -l "$KERNELSIZE" "$OUTPUT.kernel" "$KERNELDIR"
dd if="$OUTPUT.kernel" of="$OUTPUT" bs=512 seek="$KERNELOFFSET" conv=notrunc
rm -f "$OUTPUT.kernel"
