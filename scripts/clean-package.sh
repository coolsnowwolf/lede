#!/usr/bin/env bash
[ -n "$1" -a -n "$2" ] || {
	echo "Usage: $0 <file> <directory>"
	exit 1
}
[ -f "$1" -a -d "$2" ] || {
	echo "File/directory not found"
	exit 1
}
cat "$1" | (
	cd "$2"
	while read entry; do
		[ -n "$entry" ] || break
		[ -f "$entry" ] && rm -f $entry
	done
)
cat "$1" | (
	cd "$2"
	while read entry; do
		[ -n "$entry" ] || break
		[ -d "$entry" ] && rmdir "$entry" > /dev/null 2>&1
	done
)
true
