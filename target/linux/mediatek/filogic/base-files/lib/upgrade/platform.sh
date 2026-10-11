REQUIRE_IMAGE_METADATA=1
RAMFS_COPY_BIN='fitblk'

asus_initial_setup()
{
	# initialize UBI if it's running on initramfs
	[ "$(rootfs_type)" = "tmpfs" ] || return 0

	ubirmvol /dev/ubi0 -N rootfs
	ubirmvol /dev/ubi0 -N rootfs_data
	ubirmvol /dev/ubi0 -N jffs2
	ubimkvol /dev/ubi0 -N jffs2 -s 0x3e000
}

xiaomi_initial_setup()
{
	# initialize UBI and setup uboot-env if it's running on initramfs
	[ "$(rootfs_type)" = "tmpfs" ] || return 0

	local mtdnum="$( find_mtd_index ubi )"
	if [ ! "$mtdnum" ]; then
		echo "unable to find mtd partition ubi"
		return 1
	fi

	local kern_mtdnum="$( find_mtd_index ubi_kernel )"
	if [ ! "$kern_mtdnum" ]; then
		echo "unable to find mtd partition ubi_kernel"
		return 1
	fi

	ubidetach -m "$mtdnum"
	ubiformat /dev/mtd$mtdnum -y

	ubidetach -m "$kern_mtdnum"
	ubiformat /dev/mtd$kern_mtdnum -y

	if ! fw_printenv -n flag_try_sys2_failed &>/dev/null; then
		echo "failed to access u-boot-env. skip env setup."
		return 0
	fi

	fw_setenv boot_wait on
	fw_setenv uart_en 1
	fw_setenv flag_boot_rootfs 0
	fw_setenv flag_last_success 1
	fw_setenv flag_boot_success 1
	fw_setenv flag_try_sys1_failed 8
	fw_setenv flag_try_sys2_failed 8

	local board=$(board_name)
	case "$board" in
	xiaomi,redmi-router-ax6000-stock)
		fw_setenv mtdparts "nmbm0:1024k(bl2),256k(Nvram),256k(Bdata),2048k(factory),2048k(fip),256k(crash),256k(crash_log),30720k(ubi),30720k(ubi1),51200k(overlay)"
		;;
	esac
}

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	tplink,tl-7dr7299-v1)
		[ -e /dev/fit0 ] && fitblk /dev/fit0
		[ -e /dev/fitrw ] && fitblk /dev/fitrw
		CI_KERNPART="fit"
		nand_do_upgrade "$1"
		;;
	asus,tuf-ax4200|\
	asus,tuf-ax6000)
		CI_UBIPART="UBI_DEV"
		CI_KERNPART="linux"
		nand_do_upgrade "$1"
		;;
	bananapi,bpi-r3|\
	bananapi,bpi-r4|\
	bananapi,bpi-r4-2g5|\
	bananapi,bpi-r4-poe|\
	bananapi,bpi-r4-lite)
		[ -e /dev/fit0 ] && fitblk /dev/fit0
		[ -e /dev/fitrw ] && fitblk /dev/fitrw
		bootdev="$(fitblk_get_bootdev)"
		case "$bootdev" in
		mmc*)
			EMMC_KERN_DEV="/dev/$bootdev"
			emmc_do_upgrade "$1"
			;;
		mtdblock*)
			PART_NAME="/dev/mtd${bootdev:8}"
			default_do_upgrade "$1"
			;;
		ubiblock*)
			CI_KERNPART="fit"
			nand_do_upgrade "$1"
			;;
		esac
		;;
	cmcc,rax3000m-emmc|\
	cmcc,rax3000me-emmc|\
	cmcc,xr30-emmc|\
	clx,s20l|\
	glinet,gl-mt2500|\
	glinet,gl-mt6000|\
	glinet,gl-x3000|\
	glinet,gl-xe3000|\
	hf,m7986r1-emmc|\
	huasifei,wh3000-emmc|\
	huasifei,wh3000-pro|\
	jdcloud,re-cs-05)
		CI_KERNPART="kernel"
		CI_ROOTPART="rootfs"
		emmc_do_upgrade "$1"
		;;
	xiaomi,redmi-router-ax6000-stock)
		CI_KERN_UBIPART=ubi_kernel
		CI_ROOT_UBIPART=ubi
		nand_do_upgrade "$1"
		;;
	*)
		nand_do_upgrade "$1"
		;;
	esac
}

PART_NAME=firmware

platform_check_image() {
	local board=$(board_name)
	local magic="$(get_magic_long "$1")"

	[ "$#" -gt 1 ] && return 1

	case "$board" in
	tplink,tl-7dr7299-v1|\
	bananapi,bpi-r3|\
	bananapi,bpi-r4|\
	bananapi,bpi-r4-2g5|\
	bananapi,bpi-r4-poe|\
	bananapi,bpi-r4-lite)
		[ "$magic" != "d00dfeed" ] && {
			echo "Invalid image type."
			return 1
		}
		return 0
		;;
	*)
		nand_do_platform_check "$board" "$1"
		return 0
		;;
	esac

	return 0
}

platform_copy_config() {
	case "$(board_name)" in
	bananapi,bpi-r3|\
	bananapi,bpi-r4|\
	bananapi,bpi-r4-2g5|\
	bananapi,bpi-r4-poe|\
	bananapi,bpi-r4-lite)
		case "$(cmdline_get_var root)" in
		/dev/mmc*)
			emmc_copy_config
			;;
		esac
		;;
	cmcc,rax3000m-emmc|\
	cmcc,rax3000me-emmc|\
	cmcc,xr30-emmc|\
	clx,s20l|\
	glinet,gl-mt2500|\
	glinet,gl-mt6000|\
	glinet,gl-x3000|\
	glinet,gl-xe3000|\
	hf,m7986r1-emmc|\
	huasifei,wh3000-emmc|\
	huasifei,wh3000-pro|\
	jdcloud,re-cs-05)
		emmc_copy_config
		;;
	esac
}

platform_pre_upgrade() {
	local board=$(board_name)

	case "$board" in
	asus,tuf-ax4200)
		asus_initial_setup
		;;
	xiaomi,redmi-router-ax6000-stock)
		xiaomi_initial_setup
		;;
	esac
}
