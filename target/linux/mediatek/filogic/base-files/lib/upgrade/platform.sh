REQUIRE_IMAGE_METADATA=1

redmi_ax6000_nand_upgrade_tar()
{
	CI_UBIPART=ubi
	local tar_file="$1"
	local board_dir="$(tar tf "$tar_file" | grep -m 1 '^sysupgrade-.*/$')"
	board_dir="${board_dir%/}"

	local kernel_length=$( (tar xf "$tar_file" "$board_dir/kernel" -O | wc -c) 2> /dev/null)
	[ "$kernel_length" = 0 ] && kernel_length=

	local rootfs_length=$( (tar xf "$tar_file" "$board_dir/root" -O | wc -c) 2> /dev/null)
	[ "$rootfs_length" = 0 ] && rootfs_length=

	local rootfs_type
	[ "$rootfs_length" ] && rootfs_type="$(identify_tar "$tar_file" "$board_dir/root")"

	[ -n "$rootfs_length" -o -n "$kernel_length" ] || return 1

	local mtdnum="$( find_mtd_index "$CI_UBIPART" )"
	if [ ! "$mtdnum" ]; then
		echo "cannot find ubi mtd partition ubi"
		return 1
	fi
	local ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	#cleanup old data volume if exist
	if [ "$ubidev" ] && [ "$( nand_find_volume $ubidev data )" ]; then
		ubidetach -m "$mtdnum"
		ubiformat /dev/mtd$mtdnum -y
		ubiattach -m "$mtdnum"
		ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	fi
	if [ ! "$ubidev" ]; then
		ubiattach -m "$mtdnum"
		ubidev="$( nand_find_ubi "$CI_UBIPART" )"
		if [ ! "$ubidev" ]; then
			ubiformat /dev/mtd$mtdnum -y
			ubiattach -m "$mtdnum"
			ubidev="$( nand_find_ubi "$CI_UBIPART" )"

			if [ ! "$ubidev" ]; then
				echo "cannot attach ubi mtd partition ubi"
				return 1
			fi
		fi
	fi

	local kern_mtdnum="$( find_mtd_index "ubi_kernel" )"
	if [ ! "$kern_mtdnum" ]; then
		echo "cannot find ubi_kernel mtd partition ubi_kernel"
		return 1
	fi
	local kern_ubidev="$( nand_find_ubi "ubi_kernel" )"
	if [ ! "$kern_ubidev" ]; then
		ubiattach -m "$kern_mtdnum"
		kern_ubidev="$( nand_find_ubi "ubi_kernel" )"
		if [ ! "$kern_ubidev" ]; then
			ubiformat /dev/mtd$kern_mtdnum -y
			ubiattach -m "$kern_mtdnum"
			kern_ubidev="$( nand_find_ubi "ubi_kernel" )"
			if [ ! "$kern_ubidev" ]; then
				echo "cannot attach ubi_kernel mtd partition ubi_kernel"
				return 1
			fi
		fi
	fi

	local kern_ubivol="$( nand_find_volume $kern_ubidev "kernel" )"
	local root_ubivol="$( nand_find_volume $ubidev "rootfs" )"
	local data_ubivol="$( nand_find_volume $ubidev rootfs_data )"

	[ "$kern_ubivol" ] && { nand_remove_ubiblock $kern_ubivol || return 1; }
	[ "$root_ubivol" ] && { nand_remove_ubiblock $root_ubivol || return 1; }
	[ "$data_ubivol" ] && { nand_remove_ubiblock $data_ubivol || return 1; }

	[ "$data_ubivol" ] && ubirmvol /dev/$ubidev -N rootfs_data || :
	[ "$root_ubivol" ] && ubirmvol /dev/$ubidev -N "rootfs" || :
	ubirmvol /dev/$kern_ubidev -N rootfs_data 2>/dev/null || :
	ubirmvol /dev/$kern_ubidev -N rootfs 2>/dev/null || :
	[ "$kern_ubivol" ] && ubirmvol /dev/$kern_ubidev -N "kernel" || :

	# create kernel vol in ubi_kernel
	if ! ubimkvol /dev/$kern_ubidev -N "kernel" -s $kernel_length; then
		echo "cannot create kernel volume"
		return 1
	fi

	# create rootfs vol in ubi
	local rootfs_size_param
	if [ "$rootfs_type" = "ubifs" ]; then
		rootfs_size_param="-m"
	else
		rootfs_size_param="-s $rootfs_length"
	fi
	if ! ubimkvol /dev/$ubidev -N "rootfs" $rootfs_size_param; then
		echo "cannot create rootfs volume"
		return 1;
	fi

	# create rootfs_data vol for non-ubifs rootfs in ubi
	if [ "$rootfs_type" != "ubifs" ]; then
		local rootfs_data_size_param="-m"
		if ! ubimkvol /dev/$ubidev -N rootfs_data $rootfs_data_size_param; then
			if ! ubimkvol /dev/$ubidev -N rootfs_data -m; then
				echo "cannot initialize rootfs_data volume"
				return 1
			fi
		fi
	fi

	root_ubivol="$( nand_find_volume $ubidev "rootfs" )"
	if [ "$root_ubivol" ]; then
		tar xf "$tar_file" "$board_dir/root" -O | \
			ubiupdatevol /dev/$root_ubivol -s $rootfs_length -
	fi

	kern_ubivol="$( nand_find_volume $kern_ubidev "kernel" )"
	if [ "$kern_ubivol" ]; then
		tar xf "$tar_file" "$board_dir/kernel" -O | \
			ubiupdatevol /dev/$kern_ubivol -s $kernel_length -
	fi

	nand_do_upgrade_success
}

platform_do_upgrade() {
	local board=$(board_name)
	local file_type=$(identify $1)

	case "$board" in
	bananapi,bpi-r3)
		export_bootdevice
		export_partdevice rootdev 0
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
	xiaomi,redmi-router-ax6000)
		redmi_ax6000_nand_upgrade_tar "$1"
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
		export_bootdevice
		export_partdevice rootdev 0
		case "$rootdev" in
		mmc*)
			emmc_copy_config
			;;
		esac
		;;
	esac
}
