PART_NAME=dtb:kernel:rootfs
REQUIRE_IMAGE_METADATA=1

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	e2000q-demo-board |\
	e2000d-demo-board |\
	e2000q-miniitx-board |\
	e2000d-miniitx-board)
		default_do_upgrade "$1"
		;;
	esac
}

platform_check_image() {
	local board=$(board_name)
	local magic="$(get_magic_long "$1")"

	case "$board" in
	e2000q-demo-board|\
	e2000d-demo-board|\
	e2000q-miniitx-board|\
	e2000d-miniitx-board)
		[ "$magic" != "d00dfeed" ] && {
			echo "Invalid image type."
			return 1
		}
		return 0
		;;
	esac

	return 0
}
