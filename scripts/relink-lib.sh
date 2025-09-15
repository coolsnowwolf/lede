#!/bin/sh
[ $# -lt 4 -o -z "$1" -o -z "$2" -o -z "$3" -o -z "$4" ] && {
	echo "Usage: $0 <cross> <reference> <pic .a> <destination>"
	exit 1
}

cross="$1"; shift
ref="$1"; shift
pic="$1"; shift
dest="$1"; shift

SYMBOLS="$(${cross}nm "$ref" | grep -E '........ [TW] ' | awk '$3 {printf "-u%s ", $3}')"
set -x
${cross}gcc -nostdlib -nostartfiles -shared -Wl,--gc-sections -o "$dest" $SYMBOLS "$pic" "$@"
