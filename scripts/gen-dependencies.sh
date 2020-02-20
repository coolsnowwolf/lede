#!/bin/sh
#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
SELF=${0##*/}

READELF="${READELF:-readelf}"
OBJCOPY="${OBJCOPY:-objcopy}"
TARGETS=$*
XARGS="${XARGS:-xargs -r}"

[ -z "$TARGETS" ] && {
  echo "$SELF: no directories / files specified"
  echo "usage: $SELF [PATH...]"
  exit 1
}

find $TARGETS -type f -a -exec file {} \; | \
  sed -n -e 's/^\(.*\):.*ELF.*\(executable\|shared object\).*,.*/\1/p' | \
  $XARGS -n1 $READELF -d | \
  awk '$2 ~ /NEEDED/ && $NF !~ /interpreter/ && $NF ~ /^\[?lib.*\.so/ { gsub(/[\[\]]/, "", $NF); print $NF }' | \
  sort -u

tmp=$(mktemp $TMP_DIR/dep.XXXXXXXX)
for kmod in $(find $TARGETS -type f -name \*.ko); do
	$OBJCOPY -O binary -j .modinfo $kmod $tmp
	sed -e 's,\x00,\n,g' $tmp | \
		sed -ne '/^depends=.\+/ { s/^depends=//; s/,/.ko\n/g; s/$/.ko/p; q }'
done | sort -u
rm -f $tmp
