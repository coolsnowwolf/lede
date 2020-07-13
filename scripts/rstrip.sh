#!/usr/bin/env bash
# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
SELF=${0##*/}

[ -z "$STRIP" ] && {
  echo "$SELF: strip command not defined (STRIP variable not set)"
  exit 1
}

TARGETS=$*

[ -z "$TARGETS" ] && {
  echo "$SELF: no directories / files specified"
  echo "usage: $SELF [PATH...]"
  exit 1
}

find $TARGETS -type f -a -exec file {} \; | \
  sed -n -e 's/^\(.*\):.*ELF.*\(executable\|relocatable\|shared object\).*,.*/\1:\2/p' | \
(
  IFS=":"
  while read F S; do
    echo "$SELF: $F: $S"
	[ "${S}" = "relocatable" ] && {
		eval "$STRIP_KMOD $F"
	} || {
		b=$(stat -c '%a' $F)
		[ -z "$PATCHELF" ] || [ -z "$TOPDIR" ] || {
			old_rpath="$($PATCHELF --print-rpath $F)"; new_rpath=""
			for path in $old_rpath; do
				case "$path" in
					/lib/[^/]*|/usr/lib/[^/]*|\$ORIGIN/*|\$ORIGIN) new_rpath="${new_rpath:+$new_rpath:}$path" ;;
					*) echo "$SELF: $F: removing rpath $path" ;;
				esac
			done
			[ "$new_rpath" = "$old_rpath" ] || $PATCHELF --set-rpath "$new_rpath" $F
		}
		eval "$STRIP $F"
		a=$(stat -c '%a' $F)
		[ "$a" = "$b" ] || chmod $b $F
	}
  done
  true
)
