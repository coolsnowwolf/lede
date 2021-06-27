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

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	alfa-network,awusfree1)
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
	tplink,archer-c50-v4|\
	tplink,archer-c20-v5)
		MTD_ARGS="-t romfile"
		default_do_upgrade "$1"
		;;
	*)
		default_do_upgrade "$1"
		;;
	esac
}
