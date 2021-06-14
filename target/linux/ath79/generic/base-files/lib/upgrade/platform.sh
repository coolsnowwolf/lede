#
# Copyright (C) 2011 OpenWrt.org
#

PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1

RAMFS_COPY_BIN='fw_printenv fw_setenv'
RAMFS_COPY_DATA='/etc/fw_env.config /var/lock/fw_printenv.lock'

redboot_fis_do_upgrade() {
	local append
	local sysup_file="$1"
	local kern_part="$2"
	local magic=$(get_magic_word "$sysup_file")

	if [ "$magic" = "4349" ]; then
		local kern_length=0x$(dd if="$sysup_file" bs=2 skip=1 count=4 2>/dev/null)

		[ -f "$CONF_TAR" ] && append="-j $CONF_TAR"
		dd if="$sysup_file" bs=64k skip=1 2>/dev/null | \
			mtd -r $append -F$kern_part:$kern_length:0x80060000,rootfs write - $kern_part:rootfs

	elif [ "$magic" = "7379" ]; then
		local board_dir=$(tar tf $sysup_file | grep -m 1 '^sysupgrade-.*/$')
		local kern_length=$(tar xf $sysup_file ${board_dir}kernel -O | wc -c)

		[ -f "$CONF_TAR" ] && append="-j $CONF_TAR"
		tar xf $sysup_file ${board_dir}kernel ${board_dir}root -O | \
			mtd -r $append -F$kern_part:$kern_length:0x80060000,rootfs write - $kern_part:rootfs

	else
		echo "Unknown image, aborting!"
		return 1
	fi
}

platform_check_image() {
	return 0
}

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	adtran,bsap1800-v2|\
	adtran,bsap1840)
		redboot_fis_do_upgrade "$1" vmlinux_2
		;;
	allnet,all-wap02860ac|\
	engenius,eap1200h|\
	engenius,eap300-v2|\
	engenius,eap600|\
	engenius,ecb600|\
	engenius,ens202ext-v1|\
	engenius,enstationac-v1)
		IMAGE_LIST="tar tzf $1"
		IMAGE_CMD="tar xzOf $1"
		KERNEL_PART="loader"
		ROOTFS_PART="fwconcat0"
		KERNEL_FILE="uImage-lzma.bin"
		ROOTFS_FILE="root.squashfs"
		platform_do_upgrade_failsafe_datachk "$1"
		;;
	jjplus,ja76pf2)
		redboot_fis_do_upgrade "$1" linux
		;;
	openmesh,a40|\
	openmesh,a60|\
	openmesh,om2p-v2|\
	openmesh,om2p-v4|\
	openmesh,om2p-hs-v1|\
	openmesh,om2p-hs-v2|\
	openmesh,om2p-hs-v3|\
	openmesh,om2p-hs-v4|\
	openmesh,om2p-lc|\
	openmesh,om5p)
		PART_NAME="inactive"
		platform_do_upgrade_openmesh "$1"
		;;
	plasmacloud,pa300|\
	plasmacloud,pa300e)
		PART_NAME="inactive"
		platform_do_upgrade_dualboot_datachk "$1"
		;;
	ubnt,routerstation|\
	ubnt,routerstation-pro)
		redboot_fis_do_upgrade "$1" kernel
		;;
	*)
		default_do_upgrade "$1"
		;;
	esac
}
