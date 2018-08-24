#
# Copyright (C) 2011 OpenWrt.org
#

PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1

platform_check_image() {
	return 0
}

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	buffalo,bhr-4grv2)
		PART_NAME="rootfs:kernel"
		default_do_upgrade "$ARGV"
		;;
	*)
		default_do_upgrade "$ARGV"
		;;
	esac
}
