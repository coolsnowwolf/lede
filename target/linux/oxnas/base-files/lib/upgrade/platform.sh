#
# Copyright (C) 2014 OpenWrt.org
#

REQUIRE_IMAGE_METADATA=1

platform_check_image() {
	local board=$(board_name)

	[ "$ARGC" -gt 1 ] && return 1

	nand_do_platform_check $board $1
	return $?
}

platform_pre_upgrade() {
	nand_do_upgrade $1
}

disable_watchdog() {
	killall watchdog
	( ps | grep -v 'grep' | grep '/dev/watchdog' ) && {
		echo 'Could not disable watchdog'
		return 1
	}
}

append sysupgrade_pre_upgrade disable_watchdog
