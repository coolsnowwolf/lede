#!/usr/bin/env bash
#
# Copyright 2018 NXP
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

set -x
[ $# -eq 3 ] || {
    echo "SYNTAX: $0 <file> <rootfs part offset> <rootfs size>"
    exit 1
}

OUTPUT="$1"
ROOTFSOFFSET="$(($2 * 1024))"
ROOTFSSIZE="$3"

head=4
sect=16

set `ptgen -o $OUTPUT -h $head -s $sect -l $ROOTFSOFFSET -t 83 -p ${ROOTFSSIZE}M`
