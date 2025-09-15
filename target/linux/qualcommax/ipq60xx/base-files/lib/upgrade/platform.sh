PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1

RAMFS_COPY_BIN='fw_printenv fw_setenv head'
RAMFS_COPY_DATA='/etc/fw_env.config /var/lock/fw_printenv.lock'

platform_check_image() {
	return 0;
}

platform_do_upgrade() {
	case "$(board_name)" in
	cmiot,ax18|\
	glinet,gl-ax1800|\
	glinet,gl-axt1800|\
	qihoo,360v6|\
	xiaomi,rm1800)
		nand_do_upgrade "$1"
		;;
	jdcloud,re-cs-02|\
	jdcloud,re-cs-07|\
	jdcloud,re-ss-01|\
	redmi,ax5-jdcloud)
		kernelname="0:HLOS"
		rootfsname="rootfs"
		mmc_do_upgrade "$1"
		;;
	linksys,mr7350)
		boot_part="$(fw_printenv -n boot_part)"
		if [ "$boot_part" -eq "1" ]; then
			fw_setenv boot_part 2
			CI_KERNPART="alt_kernel"
			CI_UBIPART="alt_rootfs"
		else
			fw_setenv boot_part 1
			CI_UBIPART="rootfs"
		fi
		fw_setenv boot_part_ready 3
		fw_setenv auto_recovery yes
		nand_do_upgrade "$1"
		;;
	*)
		default_do_upgrade "$1"
		;;
	esac
}
