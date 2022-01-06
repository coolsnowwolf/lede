#!/bin/sh
#
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

# Write image header followed by all specified files
# The header is padded to 64k, format is:
#  CE               magic word ("Combined Extended Image") (2 bytes)
#  <CE_VERSION>     file format version field (2 bytes) 
#  <TYPE>           short description of the target device (32 bytes)
#  <NUM FILES>      number of files following the header (2 byte)
#  <file1_name>     name of the first file (32 bytes)
#  <file1_length>   length of the first file encoded as zero padded 8 digit hex (8 bytes)
#  <file1_md5>      md5 checksum of the first file (32 bytes)
#  <fileN_name>     name of the Nth file (32 bytes)
#  <fileN_length>   length of the Nth file encoded as zero padded 8 digit hex (8 bytes)
#  <fileN_md5>      md5 checksum of the Nth file (32 bytes)

## version history
# * version 1: initial file format with num files / name / length / md5 checksum 

set -e

ME="${0##*/}"

usage() {
	echo "Usage: $ME <type> <ext filename> <file1> <filename1> [<file2> <filename2> <fileN> <filenameN>]"
	[ "$IMG_OUT" ] && rm -f "$IMG_OUT"
	exit 1
}

[ "$#" -lt 4 ] && usage

CE_VERSION=1
IMG_TYPE=$1; shift
IMG_OUT=$1; shift
FILE_NUM=$(($# / 2))
FILES=""

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

printf "CE%02x%-32s%02x" $CE_VERSION "$IMG_TYPE" $FILE_NUM > "${IMG_TMP_OUT}"

while [ "$#" -gt 1 ]
   do
      file=$1
      filename=$2

      [ ! -f "$file" ] && echo "$ME: Not a valid file: $file" && usage
      FILES="$FILES $file"
      md5=$($MKHASH md5 "$file")
      printf "%-32s%08x%32s" "$filename" $(stat -c "%s" "$file") "${md5%% *}" >> "${IMG_TMP_OUT}"
      shift 2
   done

[ "$#" -eq 1 ] && echo "$ME: Filename not specified: $1" && usage

mv "${IMG_TMP_OUT}" "${IMG_TMP_OUT}".tmp
dd if="${IMG_TMP_OUT}.tmp" of="${IMG_TMP_OUT}" bs=65536 conv=sync 2>/dev/null
rm "${IMG_TMP_OUT}".tmp

cat $FILES >> "${IMG_TMP_OUT}"
cp "${IMG_TMP_OUT}" "${IMG_OUT}"
