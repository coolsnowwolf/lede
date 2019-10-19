REQUIRE_IMAGE_METADATA=1

platform_check_image() {
	local board=$(board_name)

	case "$board" in
	dlink,dir-685)
		return 0
		;;
	esac

	echo "Sysupgrade is not yet supported on $board."
	return 1
}

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	dlink,dir-685)
		PART_NAME=firmware
		default_do_upgrade "$1"
		;;
	esac
}
