#!/bin/sh
#
# Copyright (C) 2018 Oceanic Systems (UK) Ltd
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# Maintained by: Ryan Pannell <ryan [at] o s u k l .com> <github.com/Escalion>
#
# Write Linksys signature for factory image
# This is appended to the factory image and is tested by the Linksys Upgrader - as observed in civic.
# The footer is 256 bytes. The format is:
#  .LINKSYS.        This is detected by the Linksys upgrader before continuing with upgrade. (9 bytes)
#  <VERSION>        The version number of upgrade. Not checked so use arbitrary value (8 bytes)
#  <TYPE>           Model of target device, padded (0x20) to (15 bytes)
#  <CRC>      	    CRC checksum of the image to flash (8 byte)
#  <padding>	    Padding ('0' + 0x20 *7) (8 bytes)
#  <signature>	    Signature of signer. Not checked so use arbitrary value (16 bytes)
#  <padding>        Padding (0x00) (192 bytes)
#  0x0A		    (1 byte)

## version history
# * version 1: initial commit

set -e

ME="${0##*/}"

usage() {
	echo "Usage: $ME <type> <in filename>"
	[ "$IMG_OUT" ] && rm -f "$IMG_OUT"
	exit 1
}

[ "$#" -lt 3 ] && usage

TYPE=$1

tmpdir="$( mktemp -d 2> /dev/null )"
if [ -z "$tmpdir" ]; then
	# try OSX signature
	tmpdir="$( mktemp -t 'ubitmp' -d )"
fi

if [ -z "$tmpdir" ]; then
	exit 1
fi

trap "rm -rf $tmpdir" EXIT

IMG_TMP_OUT="${tmpdir}/out"

IMG_IN=$2
IMG_OUT="${IMG_IN}.new"

[ ! -f "$IMG_IN" ] && echo "$ME: Not a valid image: $IMG_IN" && usage

dd if="${IMG_IN}" of="${IMG_TMP_OUT}"
CRC=$(printf "%08X" $(dd if="${IMG_IN}" bs=$(stat -c%s "${IMG_IN}") count=1|cksum| cut -d ' ' -f1))

printf ".LINKSYS.01000409%-15s%-8s%-8s%-16s" "${TYPE}" "${CRC}" "0" "K0000000F0246434" >> "${IMG_TMP_OUT}"

dd if=/dev/zero bs=1 count=192 conv=notrunc >> "${IMG_TMP_OUT}"

printf '\12' >> "${IMG_TMP_OUT}"

cp "${IMG_TMP_OUT}" "${IMG_OUT}"
