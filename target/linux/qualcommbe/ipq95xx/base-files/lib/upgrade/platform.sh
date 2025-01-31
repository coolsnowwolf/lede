PART_NAME=firmware

RAMFS_COPY_BIN='fw_printenv fw_setenv head'
RAMFS_COPY_DATA='/etc/fw_env.config /var/lock/fw_printenv.lock'

platform_do_upgrade() {
	case "$(board_name)" in
	*)
		default_do_upgrade "$1"
		;;
	esac
}
