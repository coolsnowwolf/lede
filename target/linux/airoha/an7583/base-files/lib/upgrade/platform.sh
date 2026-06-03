RAMFS_COPY_BIN='fitblk fit_check_sign'

REQUIRE_IMAGE_METADATA=1

nokia_initial_setup()
{
	[ "$(rootfs_type)" = "tmpfs" ] || return 0

	fw_setenv bootcmd "flash read 0xc0000 0x800000 0x85000000; bootm 0x85000000"
}

platform_check_image() {
	local board=$(board_name)

	case "$board" in
	*)
		nand_do_platform_check "$board" "$1"
		return $?
		;;
	esac

	return 0
}

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	*)
		nand_do_upgrade "$1"
		;;
	esac
}

platform_pre_upgrade() {
	local board=$(board_name)

	case "$board" in
	nokia,xg-040g-mf)
		nokia_initial_setup
		;;
	*)
		;;
	esac
}
