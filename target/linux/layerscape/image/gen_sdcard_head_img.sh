#!/bin/sh
#
# Copyright 2018 NXP
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

set -x
[ $# -eq 5 ] || {
    echo "SYNTAX: $0 <file> <kernel part offset> <kernel size> <rootfs part offset> <rootfs size>"
    exit 1
}

OUTPUT="$1"
KERNELOFFSET="$(($2 * 1024))"
KERNELSIZE="$3"
ROOTFSOFFSET="$(($4 * 1024))"
ROOTFSSIZE="$5"

head=4
sect=63

set $(ptgen -o $OUTPUT -h $head -s $sect -t 83 -p ${KERNELSIZE}M@${KERNELOFFSET} -p ${ROOTFSSIZE}M@${ROOTFSOFFSET})
