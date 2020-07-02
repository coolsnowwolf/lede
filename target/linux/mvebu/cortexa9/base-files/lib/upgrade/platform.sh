#
# Copyright (C) 2014-2016 OpenWrt.org
# Copyright (C) 2016 LEDE-Project.org
#

RAMFS_COPY_BIN='fw_printenv fw_setenv'
RAMFS_COPY_DATA='/etc/fw_env.config /var/lock/fw_printenv.lock'
REQUIRE_IMAGE_METADATA=1

platform_check_image() {
	case "$(board_name)" in
	cznic,turris-omnia|\
	solidrun,clearfog-base-a1|\
	solidrun,clearfog-pro-a1)
		platform_check_image_sdcard "$1"
		;;
	*)
		return 0
		;;
	esac
}

platform_do_upgrade() {
	case "$(board_name)" in
	buffalo,ls421de)
		nand_do_upgrade "$1"
		;;
	cznic,turris-omnia|\
	solidrun,clearfog-base-a1|\
	solidrun,clearfog-pro-a1)
		platform_do_upgrade_sdcard "$1"
		;;
	linksys,wrt1200ac|\
	linksys,wrt1900ac-v1|\
	linksys,wrt1900ac-v2|\
	linksys,wrt1900acs|\
	linksys,wrt3200acm|\
	linksys,wrt32x)
		platform_do_upgrade_linksys "$1"
		;;
	*)
		default_do_upgrade "$1"
		;;
	esac
}
platform_copy_config() {
	case "$(board_name)" in
	cznic,turris-omnia|\
	solidrun,clearfog-base-a1|\
	solidrun,clearfog-pro-a1)
		platform_copy_config_sdcard
		;;
	linksys,wrt1200ac|\
	linksys,wrt1900ac-v1|\
	linksys,wrt1900ac-v2|\
	linksys,wrt1900acs|\
	linksys,wrt3200acm|\
	linksys,wrt32x)
		platform_copy_config_linksys
		;;
	esac
}
