# SPDX-License-Identifier: GPL-2.0-or-later

REQUIRE_IMAGE_METADATA=1

platform_check_image() {
	local board=$(board_name)

	case "$board" in
	netgear,wg302v1)
		return 0
		;;
	esac

	echo "Sysupgrade is not yet supported on $board."
	return 1
}

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	netgear,wg302v1)
		PART_NAME=rootfs
		default_do_upgrade "$1"
		;;
	esac
}
