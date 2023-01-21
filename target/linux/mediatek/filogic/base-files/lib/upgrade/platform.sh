REQUIRE_IMAGE_METADATA=1

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	bananapi,bpi-r3)
		local rootdev="$(cmdline_get_var root)"
		rootdev="${rootdev##*/}"
		rootdev="${rootdev%p[0-9]*}"
		case "$rootdev" in
		mmc*)
			CI_ROOTDEV="$rootdev"
			CI_KERNPART="production"
			emmc_do_upgrade "$1"
			;;
		mtdblock*)
			PART_NAME="fit"
			default_do_upgrade "$1"
			;;
		ubiblock*)
			CI_KERNPART="fit"
			nand_do_upgrade "$1"
			;;
		esac
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
	bananapi,bpi-r3)
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
	bananapi,bpi-r3)
		case "$(cmdline_get_var root)" in
		/dev/mmc*)
			emmc_copy_config
			;;
		esac
		;;
	esac
}
