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

platform_do_upgrade() {
	nand_do_upgrade $1
}
