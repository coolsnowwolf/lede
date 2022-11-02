PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1

RAMFS_COPY_BIN='fw_printenv fw_setenv'
RAMFS_COPY_DATA='/etc/fw_env.config /var/lock/fw_printenv.lock'

platform_check_image() {
	return 0;
}

platform_do_upgrade() {
	case "$(board_name)" in
	qnap,301w)
		kernelname="0:HLOS"
		rootfsname="rootfs"
		mmc_do_upgrade "$1"
		;;
	redmi,ax6|\
	xiaomi,ax3600|\
	xiaomi,ax9000)
		part_num="$(fw_printenv -n flag_boot_rootfs)"
		if [ "$part_num" -eq "1" ]; then
			CI_UBIPART="rootfs_1"
			target_num=1
			# Reset fail flag for the current partition
			# With both partition set to fail, the partition 2 (bit 1)
			# is loaded
			fw_setenv flag_try_sys2_failed 0
		else
			CI_UBIPART="rootfs"
			target_num=0
			# Reset fail flag for the current partition
			# or uboot will skip the loading of this partition
			fw_setenv flag_try_sys1_failed 0
		fi

		# Tell uboot to switch partition
		fw_setenv flag_boot_rootfs $target_num
		fw_setenv flag_last_success $target_num

		# Reset success flag
		fw_setenv flag_boot_success 0

		nand_do_upgrade "$1"
		;;
	zte,mf269)
		nand_do_upgrade "$1"
		;;
	tplink,xtr10890)
		nand_do_upgrade "$1"
		;;
	*)
		default_do_upgrade "$1"
		;;
	esac
}
