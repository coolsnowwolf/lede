#!/bin/sh
# Copyright (C) 2024 Bootlin

set -ex
[ $# -eq 8 ] || {
    echo "SYNTAX: $0 <file> <fsbl> <fip> <bootfs image> <rootfs image> <env size> <bootfs size> <rootfs size>"
    exit 1
}

OUTPUT="${1}"
FSBL="${2}"
FIP="${3}"
BOOTFS="${4}"
ROOTFS="${5}"
ENVSIZE="$((${6} / 1024))"
BOOTFSSIZE="${7}"
ROOTFSSIZE="${8}"

set $(ptgen -o "${OUTPUT}" -g -a 4 -l 2048 -G ${GUID} -N fsbla -p 2M -N fip -p 3M -N u-boot-env -p "${ENVSIZE}" -N boot -p${BOOTFSSIZE}M -N rootfs -p ${ROOTFSSIZE}M)
FSBLAOFFSET="$((${1} / 512))"
FSBLASIZE="$((${2} / 512))"
FIPOFFSET="$((${3} / 512))"
FIPSIZE="$((${4} / 512))"
ENVOFFSET="$((${5} / 512))"
ENVSIZE="$((${6} / 512))"
BOOTFSOFFSET="$((${7} / 512))"
BOOTFSSIZE="$((${8} / 512))"
ROOTFSOFFSET="$((${9} / 512))"
ROOTFSSIZE="$((${10} / 512))"

dd bs=512 if="${FSBL}" of="${OUTPUT}" seek="${FSBLAOFFSET}" conv=notrunc
dd bs=512 if="${FIP}"  of="${OUTPUT}" seek="${FIPOFFSET}" conv=notrunc
dd bs=512 if=/dev/zero of="${OUTPUT}" seek="${ENVOFFSET}" count="${ENVSIZE}" conv=notrunc
dd bs=512 if="${BOOTFS}" of="${OUTPUT}" seek="${BOOTFSOFFSET}" conv=notrunc
dd bs=512 if="${ROOTFS}" of="${OUTPUT}" seek="${ROOTFSOFFSET}" conv=notrunc
