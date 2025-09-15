#!/bin/sh
# Create a new openwrt tree with symlinks pointing at the current tree
# Usage: ./scripts/symlink-tree.sh <destination>

FILES="
	BSDmakefile
	config
	Config.in
	LICENSE
	Makefile
	README
	dl
	feeds.conf.default
	include
	package
	rules.mk
	scripts
	target
	toolchain
	tools"

OPTIONAL_FILES="
	.git"

if [ -f feeds.conf ] ; then
	FILES="$FILES feeds.conf"
fi

if [ -z "$1" ]; then
	echo "Syntax: $0 <destination>" >&2
	exit 1
fi

if [ -e "$1" ]; then
	echo "Error: $1 already exists" >&2
	exit 1
fi

set -e # fail if any commands fails
mkdir -p dl "$1"
for file in $FILES; do
	[ -e "$PWD/$file" ] || {
		echo "ERROR: $file does not exist in the current tree" >&2
		exit 1
	}
	ln -s "$PWD/$file" "$1/"
done
for file in $OPTIONAL_FILES; do
	[ -e "$PWD/$file" ] && ln -s "$PWD/$file" "$1/"
done
exit 0
