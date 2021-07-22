#
# Copyright (C) 2014-2016 OpenWrt.org
# Copyright (C) 2016 LEDE-Project.org
#

RAMFS_COPY_BIN='fw_printenv fw_setenv'
RAMFS_COPY_DATA='/etc/fw_env.config /var/lock/fw_printenv.lock'
REQUIRE_IMAGE_METADATA=1

platform_check_image() {
	case "$(board_name)" in
	glinet,gl-mv1000|\
	globalscale,espressobin|\
	globalscale,espressobin-emmc|\
	globalscale,espressobin-ultra|\
	globalscale,espressobin-v7|\
	globalscale,espressobin-v7-emmc)
		platform_check_image_sdcard "$1"
		;;
	*)
		return 0
		;;
	esac
}

platform_do_upgrade() {
	case "$(board_name)" in
	glinet,gl-mv1000|\
	globalscale,espressobin|\
	globalscale,espressobin-emmc|\
	globalscale,espressobin-ultra|\
	globalscale,espressobin-v7|\
	globalscale,espressobin-v7-emmc)
		platform_do_upgrade_sdcard "$1"
		;;
	methode,udpu)
		platform_do_upgrade_uDPU "$1"
		;;
	*)
		default_do_upgrade "$1"
		;;
	esac
}
platform_copy_config() {
	case "$(board_name)" in
	glinet,gl-mv1000|\
	globalscale,espressobin|\
	globalscale,espressobin-emmc|\
	globalscale,espressobin-ultra|\
	globalscale,espressobin-v7|\
	globalscale,espressobin-v7-emmc)
		platform_copy_config_sdcard
		;;
	methode,udpu)
		platform_copy_config_uDPU
		;;
	esac
}
