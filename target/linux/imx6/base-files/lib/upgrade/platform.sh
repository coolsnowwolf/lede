#
# Copyright (C) 2010-2015 OpenWrt.org
#

. /lib/imx6.sh

RAMFS_COPY_BIN='blkid jffs2reset'

enable_image_metadata_check() {
	case "$(board_name)" in
		toradex,apalis_imx6q-eval |\
		toradex,apalis_imx6q-ixora |\
		toradex,apalis_imx6q-ixora-v1.1 )
			REQUIRE_IMAGE_METADATA=1
			;;
	esac
}
enable_image_metadata_check

apalis_copy_config() {
	apalis_mount_boot
	cp -af "$UPGRADE_BACKUP" "/boot/$BACKUP_FILE"
	sync
	umount /boot
}

apalis_do_upgrade() {
	apalis_mount_boot
	get_image "$1" | tar Oxf - sysupgrade-apalis/kernel > /boot/uImage
	get_image "$1" | tar Oxf - sysupgrade-apalis/root > $(rootpart_from_uuid)
	sync
	umount /boot
}

platform_check_image() {
	local board=$(board_name)

	case "$board" in
	gw,imx6dl-gw51xx |\
	gw,imx6dl-gw52xx |\
	gw,imx6dl-gw53xx |\
	gw,imx6dl-gw54xx |\
	gw,imx6dl-gw551x |\
	gw,imx6dl-gw552x |\
	gw,imx6dl-gw553x |\
	gw,imx6dl-gw5904 |\
	gw,imx6dl-gw5907 |\
	gw,imx6dl-gw5910 |\
	gw,imx6dl-gw5912 |\
	gw,imx6dl-gw5913 |\
	gw,imx6q-gw51xx |\
	gw,imx6q-gw52xx |\
	gw,imx6q-gw53xx |\
	gw,imx6q-gw5400-a |\
	gw,imx6q-gw54xx |\
	gw,imx6q-gw551x |\
	gw,imx6q-gw552x |\
	gw,imx6q-gw553x |\
	gw,imx6q-gw5904 |\
	gw,imx6q-gw5907 |\
	gw,imx6q-gw5910 |\
	gw,imx6q-gw5912 |\
	gw,imx6q-gw5913 )
		nand_do_platform_check $board $1
		return $?;
		;;
	toradex,apalis_imx6q-eval |\
	toradex,apalis_imx6q-ixora |\
	toradex,apalis_imx6q-ixora-v1.1 )
		return 0
		;;
	esac

	echo "Sysupgrade is not yet supported on $board."
	return 1
}

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	gw,imx6dl-gw51xx |\
	gw,imx6dl-gw52xx |\
	gw,imx6dl-gw53xx |\
	gw,imx6dl-gw54xx |\
	gw,imx6dl-gw551x |\
	gw,imx6dl-gw552x |\
	gw,imx6dl-gw553x |\
	gw,imx6dl-gw5904 |\
	gw,imx6dl-gw5907 |\
	gw,imx6dl-gw5910 |\
	gw,imx6dl-gw5912 |\
	gw,imx6dl-gw5913 |\
	gw,imx6q-gw51xx |\
	gw,imx6q-gw52xx |\
	gw,imx6q-gw53xx |\
	gw,imx6q-gw5400-a |\
	gw,imx6q-gw54xx |\
	gw,imx6q-gw551x |\
	gw,imx6q-gw552x |\
	gw,imx6q-gw553x |\
	gw,imx6q-gw5904 |\
	gw,imx6q-gw5907 |\
	gw,imx6q-gw5910 |\
	gw,imx6q-gw5912 |\
	gw,imx6q-gw5913 )
		nand_do_upgrade "$1"
		;;
	toradex,apalis_imx6q-eval |\
	toradex,apalis_imx6q-ixora |\
	toradex,apalis_imx6q-ixora-v1.1 )
		apalis_do_upgrade "$1"
		;;
	esac
}

platform_copy_config() {
	local board=$(board_name)

	case "$board" in
	toradex,apalis_imx6q-eval |\
	toradex,apalis_imx6q-ixora |\
	toradex,apalis_imx6q-ixora-v1.1 )
		apalis_copy_config
		;;
	esac
}

platform_pre_upgrade() {
	local board=$(board_name)

	case "$board" in
	toradex,apalis_imx6q-eval |\
	toradex,apalis_imx6q-ixora |\
	toradex,apalis_imx6q-ixora-v1.1 )
		[ -z "$UPGRADE_BACKUP" ] && {
			jffs2reset -y
			umount /overlay
		}
		;;
	esac
}
