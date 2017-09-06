PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1

RAMFS_COPY_BIN='fw_printenv fw_setenv'
RAMFS_COPY_DATA='/etc/fw_env.config /var/lock/fw_printenv.lock'

platform_check_image() {
	local board=$(board_name)

	case "$board" in
	rt-ac58u)
		CI_UBIPART="UBI_DEV"
		local ubidev=$(nand_find_ubi $CI_UBIPART)
		local asus_root=$(nand_find_volume $ubidev jffs2)

		[ -n "$asus_root" ] || return 0

		cat << EOF
jffs2 partition is still present.
There's probably no space left
to install the filesystem.

You need to delete the jffs2 partition first:
# ubirmvol /dev/ubi0 --name=jffs2

Once this is done. Retry.
EOF
		return 1
		;;
	esac
	return 0
}

platform_do_upgrade() {
	case "$(board_name)" in
	ap148 |\
	ap-dk04.1-c1 |\
	d7800 |\
	nbg6817 |\
	r7500 |\
	r7500v2 |\
	r7800)
		nand_do_upgrade "$ARGV"
		;;
	c2600)
		PART_NAME="os-image:rootfs"
		MTD_CONFIG_ARGS="-s 0x200000"
		default_do_upgrade "$ARGV"
		;;
	ea8500)
		platform_do_upgrade_linksys "$ARGV"
		;;
	vr2600v)
		PART_NAME="kernel:rootfs"
		MTD_CONFIG_ARGS="-s 0x200000"
		default_do_upgrade "$ARGV"
		;;
	rt-ac58u)
		CI_UBIPART="UBI_DEV"
		CI_KERNPART="linux"
		nand_do_upgrade "$1"
		;;
	*)
		default_do_upgrade "$ARGV"
		;;
	esac
}

platform_nand_pre_upgrade() {
	case "$(board_name)" in
	nbg6817)
		zyxel_do_upgrade "$1"
		;;
	rt-ac58u)
		CI_UBIPART="UBI_DEV"
		CI_KERNPART="linux"
		;;
	esac
}

blink_led() {
	. /etc/diag.sh; set_state upgrade
}

append sysupgrade_pre_upgrade blink_led
