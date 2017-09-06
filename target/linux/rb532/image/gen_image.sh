#!/usr/bin/env bash
OUTPUT="$1"
KERNELSIZE="$2"
KERNELIMAGE="$3"
ROOTFSSIZE="$4"
ROOTFSIMAGE="$5"
ALIGN="$6"

rm -f "$OUTPUT"

# create partition table
set `ptgen -o "$OUTPUT" -h 16 -s 32 -l ${ALIGN} -t 0x27 -p ${KERNELSIZE}m -t 0x83 -p ${ROOTFSSIZE}m`

KERNELOFFSET="$(($1 / 512))"
ROOTFSOFFSET="$(($3 / 512))"

dd if="$KERNELIMAGE" of="$OUTPUT" bs=512 conv=notrunc seek="$KERNELOFFSET"
dd if="$ROOTFSIMAGE" of="$OUTPUT" bs=512 conv=notrunc seek="$ROOTFSOFFSET"
