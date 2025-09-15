#!/bin/sh


get_magic_word() {
	dd if=$1 bs=4 count=1 2>/dev/null | od -A n -N 4 -t x1 | tr -d ' '
}

get_post_padding_word() {
	local rootfs_length="$(stat -c%s "$1")"
	[ "$rootfs_length" -ge 4 ] || return
	rootfs_length=$((rootfs_length-4))

	# the JFFS2 end marker must be on a 4K boundary (often 64K or 256K)
	local unaligned_bytes=$((rootfs_length%4096))
	[ "$unaligned_bytes" = 0 ] || return

	# skip rootfs data except the potential EOF marker
	dd if="$1" bs=1 skip="$rootfs_length" 2>/dev/null | od -A n -N 4 -t x1 | tr -d ' '
}

get_fs_type() {
	local magic_word="$(get_magic_word "$1")"

	case "$magic_word" in
	"3118"*)
		echo "ubifs"
		;;
	"68737173")
		local post_padding_word="$(get_post_padding_word "$1")"

		case "$post_padding_word" in
		"deadc0de")
			echo "squashfs-jffs2"
			;;
		*)
			echo "squashfs"
			;;
		esac
		;;
	*)
		echo "unknown"
		;;
	esac
}

round_up() {
	echo "$(((($1 + ($2 - 1))/ $2) * $2))"
}
