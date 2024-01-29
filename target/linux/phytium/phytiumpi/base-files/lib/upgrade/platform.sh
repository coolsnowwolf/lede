REQUIRE_IMAGE_METADATA=1

platform_do_upgrade() {
	local board=$(board_name)
	case "$board" in
	phytiumpi_firefly)
		export_bootdevice
		export_partdevice rootdev 0
		case "$rootdev" in
		mmc*)
			CI_ROOTDEV="$rootdev"
			CI_KERNPART="kernel"
			CI_ROOTPART="rootfs"
			emmc_do_upgrade "$1"
			;;
		esac
		;;
	esac
}

platform_check_image() {
	local board=$(board_name)
	local magic="$(get_magic_long "$1")"
	case "$board" in
	phytiumpi_firefly)
		[ "$magic" != "73797375" ] && {
			echo "Invalid image type."
			return 1
		}
		return 0
		;;
	esac

	return 0
}

platform_copy_config() {
	case "$(board_name)" in
	phytiumpi_firefly)
		export_bootdevice
		export_partdevice rootdev 0
		if echo $rootdev | grep -q mmc; then
			emmc_copy_config
		fi
		;;
	esac
}
