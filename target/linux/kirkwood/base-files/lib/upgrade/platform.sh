RAMFS_COPY_BIN='fw_printenv fw_setenv'
RAMFS_COPY_DATA='/etc/fw_env.config /var/lock/fw_printenv.lock'

REQUIRE_IMAGE_METADATA=1

platform_check_image() {
	return 0
}

platform_do_upgrade() {
	local board="$(board_name)"

	case "$board" in
	"linksys,audi"|\
	"linksys,viper")
		platform_do_upgrade_linksys "$ARGV"
		;;
	*)
		nand_do_upgrade "$ARGV"
		;;
	esac
}
