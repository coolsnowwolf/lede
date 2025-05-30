#
# Copyright (C) 2014-2016 OpenWrt.org
# Copyright (C) 2016 LEDE-Project.org
#

RAMFS_COPY_BIN='fw_printenv fw_setenv seq strings'
RAMFS_COPY_DATA='/etc/fw_env.config /var/lock/fw_printenv.lock'

REQUIRE_IMAGE_METADATA=1

platform_check_image() {
	case "$(board_name)" in
	cznic,turris-omnia|\
	kobol,helios4|\
	solidrun,clearfog-base-a1|\
	solidrun,clearfog-pro-a1)
		legacy_sdcard_check_image "$1"
		;;
	*)
		return 0
		;;
	esac
}

platform_do_upgrade() {
	case "$(board_name)" in
	buffalo,ls220de)
		# Kernel UBI volume name must be "boot"
		CI_KERNPART=boot
		CI_KERN_UBIPART=ubi_kernel
		CI_ROOT_UBIPART=ubi
		nand_do_upgrade "$1"
		;;
	buffalo,ls421de|\
	wd,cloud-mirror-gen2)
		nand_do_upgrade "$1"
		;;
	ctera,c200-v2)
	part=$(find_mtd_part "active_bank")

	if [ -n "$part" ]; then
		CI_KERNPART="$(strings $part | grep bank)"
		nand_do_upgrade "$1"
	else
		echo "active_bank partition missed!"
		return 1
	fi
	;;
	cznic,turris-omnia|\
	kobol,helios4|\
	solidrun,clearfog-base-a1|\
	solidrun,clearfog-pro-a1)
		legacy_sdcard_do_upgrade "$1"
		;;
	fortinet,fg-30e|\
	fortinet,fg-50e|\
	fortinet,fg-51e|\
	fortinet,fg-52e|\
	fortinet,fwf-50e-2r|\
	fortinet,fwf-51e)
		fortinet_do_upgrade "$1"
		;;
	iij,sa-w2)
		local envmtd=$(find_mtd_part "bootloader-env")
		local bootdev=$(grep "BOOTDEV=" "$envmtd")
		case "${bootdev#*=}" in
		flash)  PART_NAME="firmware" ;;
		rescue) PART_NAME="rescue"   ;;
		*)
			echo "invalid BOOTDEV is set (\"${bootdev#*=}\")"
			umount -a
			reboot -f
			;;
		esac
		default_do_upgrade "$1"
		;;
	iptime,nas1dual)
		PART_NAME=firmware
		default_do_upgrade "$1"
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
	kobol,helios4|\
	solidrun,clearfog-base-a1|\
	solidrun,clearfog-pro-a1)
		legacy_sdcard_copy_config
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
