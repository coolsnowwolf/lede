PART_NAME=dtb:kernel:rootfs
REQUIRE_IMAGE_METADATA=1

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	e2000q-demo-board |\
	e2000d-demo-board |\
	e2000q-miniitx-board |\
	e2000d-miniitx-board)
		default_do_upgrade "$1"
		;;
	esac
}

platform_check_image() {
	local board=$(board_name)
	local magic="$(get_magic_long "$1")"

	case "$board" in
	e2000q-demo-board|\
	e2000d-demo-board|\
	e2000q-miniitx-board|\
	e2000d-miniitx-board)
		[ "$magic" != "d00dfeed" ] && {
			echo "Invalid image type."
			return 1
		}
		local kernel_size="$(awk '$4 == "\"kernel\"" { print $2 }' /proc/mtd)"
		if [ -z "$kernel_size" ] || [ "$((0x$kernel_size))" -lt 12582912 ]; then
			echo "The kernel partition must be at least 12 MiB. Install the factory image to update the partition layout."
			return 1
		fi
		return 0
		;;
	esac

	return 0
}
