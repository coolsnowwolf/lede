#
# Copyright (C) 2010 OpenWrt.org
#

PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1

RAMFS_COPY_BIN='fw_printenv fw_setenv'
RAMFS_COPY_DATA='/etc/fw_env.config /var/lock/fw_printenv.lock'

platform_check_image() {
	return 0
}

platform_pre_upgrade() {
	local board=$(board_name)

	case "$board" in
	mikrotik,rb750gr3|\
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
	ubnt-erx-sfp)
		platform_upgrade_ubnt_erx "$1"
		;;
	esac
}

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	alfa-network,ac1200rm|\
	alfa-network,awusfree1|\
	alfa-network,quad-e4g|\
	alfa-network,r36m-e4g|\
	alfa-network,tube-e4g)
		[ "$(fw_printenv -n dual_image 2>/dev/null)" = "1" ] &&\
		[ -n "$(find_mtd_part backup)" ] && {
			PART_NAME=backup
			if [ "$(fw_printenv -n bootactive 2>/dev/null)" = "1" ]; then
				fw_setenv bootactive 2 || exit 1
			else
				fw_setenv bootactive 1 || exit 1
			fi
		}
		default_do_upgrade "$1"
		;;
	hc5962|\
	r6220|\
	netgear,r6350|\
	ubnt-erx|\
	ubnt-erx-sfp|\
	xiaomi,mir3g|\
	xiaomi,mir3p)
		nand_do_upgrade "$1"
		;;
	tplink,c50-v4)
		MTD_ARGS="-t romfile"
		default_do_upgrade "$1"
		;;
	*)
		default_do_upgrade "$1"
		;;
	esac
}
