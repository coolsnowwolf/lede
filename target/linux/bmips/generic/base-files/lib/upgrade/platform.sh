# SPDX-License-Identifier: GPL-2.0-or-later

PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1

platform_check_image() {
	return 0
}

platform_do_upgrade() {
	case "$(board_name)" in
		*)
			default_do_upgrade "$1"
			;;
	esac
}
