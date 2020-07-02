PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1

RAMFS_COPY_BIN='fw_printenv fw_setenv'
RAMFS_COPY_DATA='/etc/fw_env.config /var/lock/fw_printenv.lock'

platform_check_image() {
	return 0;
}

platform_do_upgrade() {
	case "$(board_name)" in
	buffalo,wxr-2533dhp)
		buffalo_upgrade_prepare_ubi
		CI_ROOTPART="ubi_rootfs"
		nand_do_upgrade "$1"
		;;
	compex,wpq864|\
	netgear,d7800 |\
	netgear,r7500 |\
	netgear,r7500v2 |\
	netgear,r7800 |\
	qcom,ipq8064-ap148 |\
	qcom,ipq8064-ap161)
		nand_do_upgrade "$1"
		;;
	zyxel,nbg6817)
		zyxel_do_upgrade "$1"
		;;
	linksys,ea8500)
		platform_do_upgrade_linksys "$1"
		;;
	tplink,c2600)
		PART_NAME="os-image:rootfs"
		MTD_CONFIG_ARGS="-s 0x200000"
		default_do_upgrade "$1"
		;;
	tplink,vr2600v)
		PART_NAME="kernel:rootfs"
		MTD_CONFIG_ARGS="-s 0x200000"
		default_do_upgrade "$1"
		;;
	*)
		default_do_upgrade "$1"
		;;
	esac
}
