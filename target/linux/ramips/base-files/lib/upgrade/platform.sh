#
# Copyright (C) 2010 OpenWrt.org
#

PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1

platform_check_image() {
	return 0
}

platform_pre_upgrade() {
	local board=$(board_name)

	case "$board" in
	mikrotik,rbm11g|\
	mikrotik,rbm33g)
		[ -z "$(rootfs_type)" ] && mtd erase firmware
		;;
	esac
}

platform_nand_pre_upgrade() {
	local board=$(board_name)

	case "$board" in
	ubnt-erx|\
	ubnt-erx-sfp|\
	xiaomi,mir3p)
		platform_upgrade_ubnt_erx "$ARGV"
		;;
	esac
}

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	hc5962|\
	mir3g|\
	mir4|\
	r6220|\
	netgear,r6350|\
	xiaomi,miwifi-r3|\
	ubnt-erx|\
	ubnt-erx-sfp)
		nand_do_upgrade "$ARGV"
		;;
	*)
		default_do_upgrade "$ARGV"
		;;
	esac
}
