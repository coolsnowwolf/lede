#
# Copyright (C) 2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

RAMFS_COPY_BIN='fw_printenv fw_setenv dmesg'
RAMFS_COPY_DATA="/etc/fw_env.config"
REQUIRE_IMAGE_METADATA=0

platform_check_image()
{
	local board=$(board_name)

	nand_do_platform_check $board $1
	return $?
}

platform_do_upgrade() {
	# TODO no need to switch to ramfs with dual partitions in
	# fact we don't even want to reboot as part of seamless
	# upgrades. Instead just upgrade opposite partition and mark
	# the next reboot to boot from that partition. Could just call
	# stage2 directly but need to refactor nand_upgrade_success
	# for this to work.
	#   Also the nand functions don't allow url to be used

	local board=$(board_name)

	case "$board" in
	img,pistachio-marduk)
		local boot_partition=$(dmesg | grep "ubi0: attached.*" | sed "s/^.*\(firmware[0-1]\).*/\1/g")

		echo "Current boot partiton $boot_partition (/dev/mtd$(find_mtd_index $boot_partition))"
		mkdir -p /var/lock
		if [ "$boot_partition" == "firmware0" ]; then
			CI_UBIPART="firmware1"
			fw_setenv boot_partition 1 || exit 1
		else
			CI_UBIPART="firmware0"
			fw_setenv boot_partition 0 || exit 1
		fi
		echo "Upgrading partition $CI_UBIPART (/dev/mtd$(find_mtd_index $CI_UBIPART))"
		;;
	esac

	nand_do_upgrade $1
}
