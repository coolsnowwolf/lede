#
# Copyright (C) 2010-2015 OpenWrt.org
#

platform_check_image() {
	local board=$(board_name)

	case "$board" in
	*gw5*)
		nand_do_platform_check $board $1
		return $?;
		;;
	esac

	echo "Sysupgrade is not yet supported on $board."
	return 1
}

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	*gw5*)
		nand_do_upgrade "$1"
		;;
	esac
}
