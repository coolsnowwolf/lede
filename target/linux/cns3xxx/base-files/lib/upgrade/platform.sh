PART_NAME=firmware

CI_BLKSZ=65536

platform_check_image() {
	local magic="$(get_magic_long "$1")"

	[ "$#" -gt 1 ] && return 1

	[ "$magic" != "27051956" ] && {
		echo "Invalid image type."
		return 1
	}
	return 0
}

platform_do_upgrade() {
	default_do_upgrade "$1"
}
