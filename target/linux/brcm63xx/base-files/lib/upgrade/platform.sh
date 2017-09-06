PART_NAME=linux
platform_check_image() {
	[ "$#" -gt 1 ] && return 1
	case "$(get_magic_word "$1")" in
		3600|3700|3800)
			# 6348GW-11 boards use openwrt-96348GW-11-squashfs-cfe.bin files
			return 0
			;;
		*)
			echo "Invalid image type. Please use only .bin files"
			return 1
			;;
	esac
}

# use default for platform_do_upgrade()
