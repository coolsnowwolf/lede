PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1

platform_check_image() {
	return 0
}

platform_pre_upgrade() {
	local board=$(board_name)

	case "$board" in
	BTHOMEHUBV2B|BTHOMEHUBV3A|BTHOMEHUBV5A|P2812HNUF* )
		nand_do_upgrade $1
		;;
	esac
}

# use default for platform_do_upgrade()

disable_watchdog() {
	killall watchdog
	( ps | grep -v 'grep' | grep '/dev/watchdog' ) && {
		echo 'Could not disable watchdog'
		return 1
	}
}
append sysupgrade_pre_upgrade disable_watchdog
