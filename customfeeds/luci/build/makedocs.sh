#!/bin/bash

topdir=$(pwd)

[ -f "$topdir/build/makedocs.sh" -a -n "$1" ] || {
	echo "Please execute as ./build/makedocs.sh [output directory]" >&2
	exit 1
}

outdir=$(readlink -f "$1")

mkdir -p "$outdir" || {
	echo "Unable to mkdir '$outdir'" >&2
	exit 1
}

(
	cd "$topdir/build/luadoc/"
	find "$topdir/libs/" "$topdir/modules/" -type f -name '*.lua' -or -name '*.luadoc' | \
		xargs grep -l '@return' | xargs ./doc.lua --no-files -d "$outdir"
)
