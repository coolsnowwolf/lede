# Copyright (C) 2011 OpenWrt.org

PART_NAME=firmware

REQUIRE_IMAGE_METADATA=1
platform_check_image() {
	return 0
}

RAMFS_COPY_BIN='fw_printenv fw_setenv nandwrite'
RAMFS_COPY_DATA='/etc/fw_env.config /var/lock/fw_printenv.lock'

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	glinet,gl-ar300m-nand|\
	glinet,gl-ar300m-nor)
		glinet_nand_nor_do_upgrade "$1"
		;;
	glinet,gl-ar750s-nor|\
	glinet,gl-ar750s-nor-nand)
		nand_nor_do_upgrade "$1"
		;;
	*)
		nand_do_upgrade "$1"
		;;
	esac
}
