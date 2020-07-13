linksys_get_target_firmware() {
	local cur_boot_part mtd_ubi0

	cur_boot_part="$(/usr/sbin/fw_printenv -n boot_part)"
	if [ -z "${cur_boot_part}" ]; then
		mtd_ubi0=$(cat /sys/devices/virtual/ubi/ubi0/mtd_num)
		case "$(grep -E "^mtd${mtd_ubi0}:" /proc/mtd | cut -d '"' -f 2)" in
		kernel|rootfs)
			cur_boot_part=1
			;;
		alt_kernel|alt_rootfs)
			cur_boot_part=2
			;;
		esac
		>&2 printf "Current boot_part='%s' selected from ubi0/mtd_num='%s'" \
			"${cur_boot_part}" "${mtd_ubi0}"
	fi

	# OEM U-Boot for EA6350v3 and EA8300; bootcmd=
	#  if test $auto_recovery = no;
	#      then bootipq;
	#  elif test $boot_part = 1;
	#      then run bootpart1;
	#      else run bootpart2;
	#  fi

	case "$cur_boot_part" in
	1)
		fw_setenv -s - <<-EOF
			boot_part 2
			auto_recovery yes
		EOF
		printf "alt_kernel"
		return
		;;
	2)
		fw_setenv -s - <<-EOF
			boot_part 1
			auto_recovery yes
		EOF
		printf "kernel"
		return
		;;
	*)
		return
		;;
	esac
}

linksys_get_root_magic() {
	(get_image "$@" | dd skip=786432 bs=4 count=1 | hexdump -v -n 4 -e '1/1 "%02x"') 2>/dev/null
}

platform_do_upgrade_linksys() {
	local magic_long="$(get_magic_long "$1")"

	local rm_oem_fw_vols="squashfs ubifs"	# from OEM [alt_]rootfs UBI
	local vol

	mkdir -p /var/lock
	local part_label="$(linksys_get_target_firmware)"
	touch /var/lock/fw_printenv.lock

	if [ -z "$part_label" ]; then
		echo "cannot find target partition"
		exit 1
	fi

	local target_mtd=$(find_mtd_part "$part_label")

	[ "$magic_long" = "73797375" ] && {
		CI_KERNPART="$part_label"
		if [ "$part_label" = "kernel" ]; then
			CI_UBIPART="rootfs"
		else
			CI_UBIPART="alt_rootfs"
		fi

		local mtdnum="$(find_mtd_index "$CI_UBIPART")"
		if [ ! "$mtdnum" ]; then
			echo "cannot find ubi mtd partition $CI_UBIPART"
			return 1
		fi

		local ubidev="$(nand_find_ubi "$CI_UBIPART")"
		if [ ! "$ubidev" ]; then
			ubiattach -m "$mtdnum"
			sync
			ubidev="$(nand_find_ubi "$CI_UBIPART")"
		fi

		if [ "$ubidev" ]; then
			for vol in $rm_oem_fw_vols; do
				ubirmvol "/dev/$ubidev" -N "$vol" 2>/dev/null
			done
		fi

		# complete std upgrade
		nand_upgrade_tar "$1"
	}

	[ "$magic_long" = "27051956" ] && {
		# This magic is for a uImage (which is a sysupgrade image)
		# check firmwares' rootfs types
		local oldroot="$(linksys_get_root_magic "$target_mtd")"
		local newroot="$(linksys_get_root_magic "$1")"

		if [ "$newroot" = "55424923" ] && [ "$oldroot" = "55424923" ]; then
			# we're upgrading from a firmware with UBI to one with UBI
			# erase everything to be safe
			# - Is that really needed? Won't remove (or comment) the if,
			#   because it may be needed in a future device.
			#mtd erase $part_label
			#get_image "$1" | mtd -n write - $part_label
			echo "writing \"$1\" UBI image to \"$part_label\" (UBI)..."
			get_image "$1" | mtd write - "$part_label"
		else
			echo "writing \"$1\" image to \"$part_label\""
			get_image "$1" | mtd write - "$part_label"
		fi
	}
}
