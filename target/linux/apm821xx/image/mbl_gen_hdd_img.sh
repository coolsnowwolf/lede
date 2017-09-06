#!/usr/bin/env bash

set -x
[ $# -eq 3 ] || {
    echo "SYNTAX: $0 <file> <bootfs image> <rootfs image>"
    exit 1
}

OUTPUT="$1"
BOOTFS="$2"
ROOTFS="$3"
let "BOOTFSSIZE=(`stat -c%s "$2"` + 1048575) / 1048576"
let "ROOTFSSIZE=(`stat -c%s "$3"` + 1048575) / 1048576"

head=4
sect=63

set `ptgen -o $OUTPUT -h $head -s $sect -l 4096 -t 83 -p ${BOOTFSSIZE}M -t 83 -p ${ROOTFSSIZE}M`

BOOTOFFSET="$(($1 / 512))"
BOOTSIZE="$(($2 / 512))"
ROOTFSOFFSET="$(($3 / 512))"
ROOTFSSIZE="$(($4 / 512))"

dd bs=512 if="$BOOTFS" of="$OUTPUT" seek="$BOOTOFFSET" conv=notrunc
dd bs=512 if="$ROOTFS" of="$OUTPUT" seek="$ROOTFSOFFSET" conv=notrunc

