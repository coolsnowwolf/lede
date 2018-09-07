PART_NAME=linux
platform_check_image() {
	[ "$#" -gt 1 ] && return 1

	case "$(get_magic_word "$1")" in
		# u-boot
		2705) return 0;;
		*)
			echo "Invalid image type. Please use only u-boot files"
			return 1
		;;
	esac
}

# use default for platform_do_upgrade()
