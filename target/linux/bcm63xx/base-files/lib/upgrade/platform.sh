PART_NAME=linux
REQUIRE_IMAGE_METADATA=0

platform_check_image() {
	[ "$#" -gt 1 ] && return 1

	case "$(board_name)" in
		comtrend,vg-8050|\
		comtrend,vr-3032u|\
		huawei,hg253s-v2|\
		netgear,dgnd3700-v2|\
		sercomm,ad1018|\
		sercomm,h500-s-lowi|\
		sercomm,h500-s-vfes)
			# NAND sysupgrade
			return 0
			;;
	esac

	case "$(get_magic_word "$1")" in
		3600|3700|3800)
			# CFE tag versions
			return 0
			;;
		*)
			echo "Invalid image type. Please use only .bin files"
			return 1
			;;
	esac
}

cfe_jffs2_upgrade_tar() {
	local tar_file="$1"
	local kernel_mtd="$(find_mtd_index $CI_KERNPART)"

	if [ -z "$kernel_mtd" ]; then
		echo "$CI_KERNPART partition not found"
		return 1
	fi

	local board_dir=$(tar tf $tar_file | grep -m 1 '^sysupgrade-.*/$')
	board_dir=${board_dir%/}

	local kernel_length=$(tar xf $tar_file ${board_dir}/kernel -O | wc -c 2> /dev/null)
	local rootfs_length=$(tar xf $tar_file ${board_dir}/root -O | wc -c 2> /dev/null)

	if [ "$kernel_length" = 0 ]; then
		echo "kernel cannot be empty"
		return 1
	fi

	flash_erase -j /dev/mtd${kernel_mtd} 0 0
	tar xf $tar_file ${board_dir}/kernel -O | nandwrite /dev/mtd${kernel_mtd} -

	local rootfs_type="$(identify_tar "$tar_file" ${board_dir}/root)"

	nand_upgrade_prepare_ubi "$rootfs_length" "$rootfs_type" "0" "0"

	local ubidev="$( nand_find_ubi "$CI_UBIPART" )"

	local root_ubivol="$(nand_find_volume $ubidev $CI_ROOTPART)"
	tar xf $tar_file ${board_dir}/root -O | \
		ubiupdatevol /dev/$root_ubivol -s $rootfs_length -

	nand_do_upgrade_success
}

platform_do_upgrade() {
	case "$(board_name)" in
		comtrend,vg-8050|\
		comtrend,vr-3032u|\
		huawei,hg253s-v2|\
		netgear,dgnd3700-v2)
			REQUIRE_IMAGE_METADATA=1
			cfe_jffs2_upgrade_tar "$1"
			;;
		sercomm,ad1018|\
		sercomm,h500-s-lowi|\
		sercomm,h500-s-vfes)
			REQUIRE_IMAGE_METADATA=1
			nand_do_upgrade "$1"
			;;
		*)
			default_do_upgrade "$1"
			;;
	esac
}
