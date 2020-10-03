#!/bin/sh


get_magic_word() {
	dd if=$1 bs=4 count=1 2>/dev/null | od -A n -N 4 -t x1 | tr -d ' '
}

get_fs_type() {
	local magic_word="$(get_magic_word "$1")"

	case "$magic_word" in
	"3118"*)
		echo "ubifs"
		;;
	"68737173")
		echo "squashfs"
		;;
	*)
		echo "unknown"
		;;
	esac
}

round_up() {
	echo "$(((($1 + ($2 - 1))/ $2) * $2))"
}
