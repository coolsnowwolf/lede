#
# Copyright (C) 2009-2010 OpenWrt.org
#

. /lib/adm5120.sh

PART_NAME="firmware"
RAMFS_COPY_DATA=/lib/adm5120.sh

platform_check_image() {
	local magic="$(get_magic_word "$1")"

	[ "$#" -gt 1 ] && return 1

	case "$board_name" in
	"ZyXEL"*|"Compex WP54 family")
		# .trx files
		[ "$magic" != "4844" ] && {
			echo "Invalid image type."
			return 1
		}
		return 0
		;;
	*)
		;;
	esac

	echo "Sysupgrade is not yet supported on $board_name."
	return 1
}

platform_do_upgrade() {
	PART_NAME="$sys_mtd_part"
	default_do_upgrade "$ARGV"
}
