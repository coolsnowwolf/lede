linksys_get_target_firmware() {
	cur_boot_part=$(/usr/sbin/fw_printenv -n boot_part)
	target_firmware=""
	if [ "$cur_boot_part" = "1" ]; then
		# current primary boot - update alt boot
		target_firmware="alt_kernel"
		fw_setenv boot_part 2
		# In the Linksys EA6350v3, it is enough to set the boot_part as the boot command line is
		# bootcmd=if test $boot_part = 1; then run bootpart1; else run bootpart2; fi
		# - You probably want to use that if your device's uboot does not eval bootcmd
		#fw_setenv bootcmd "run altnandboot"
	elif [ "$cur_boot_part" = "2" ]; then
		# current alt boot - update primary boot
		target_firmware="kernel"
		fw_setenv boot_part 1
		#fw_setenv bootcmd "run nandboot"
	fi

	# re-enable recovery so we get back if the new firmware is broken
	fw_setenv auto_recovery yes
	# see /etc/init.d/zlinksys_recovery

	echo "$target_firmware"
}

linksys_get_root_magic() {
	(get_image "$@" | dd skip=786432 bs=4 count=1 | hexdump -v -n 4 -e '1/1 "%02x"') 2>/dev/null
}

platform_do_upgrade_linksys() {
	local magic_long="$(get_magic_long "$1")"

	mkdir -p /var/lock
	local part_label="$(linksys_get_target_firmware)"
	touch /var/lock/fw_printenv.lock

	if [ ! -n "$part_label" ]; then
		echo "cannot find target partition"
		exit 1
	fi

	local target_mtd=$(find_mtd_part $part_label)

	[ "$magic_long" = "73797375" ] && {
		CI_KERNPART="$part_label"
		if [ "$part_label" = "kernel" ]; then
			CI_UBIPART="rootfs"
		else
			CI_UBIPART="alt_rootfs"
		fi

		# remove "squashfs" vol (in case we are flashing over a stock image, which is also UBI)

		local mtdnum="$( find_mtd_index "$CI_UBIPART" )"
		if [ ! "$mtdnum" ]; then
			echo "cannot find ubi mtd partition $CI_UBIPART"
			return 1
		fi

		local ubidev="$( nand_find_ubi "$CI_UBIPART" )"
		if [ ! "$ubidev" ]; then
			ubiattach -m "$mtdnum"
			sync
			ubidev="$( nand_find_ubi "$CI_UBIPART" )"
		fi

		if [ "$ubidev" ]; then
			local squash_ubivol="$( nand_find_volume $ubidev squashfs )"
			# kill volume
			[ "$squash_ubivol" ] && ubirmvol /dev/$ubidev -N squashfs || true
		fi

		# complete std upgrade
		nand_upgrade_tar "$1"
	}
	[ "$magic_long" = "27051956" ] && {
		# This magic is for a uImage (which is a sysupgrade image)
		# check firmwares' rootfs types
		local oldroot="$(linksys_get_root_magic $target_mtd)"
		local newroot="$(linksys_get_root_magic "$1")"

		if [ "$newroot" = "55424923" -a "$oldroot" = "55424923" ]; then
			# we're upgrading from a firmware with UBI to one with UBI
			# erase everything to be safe
			# - Is that really needed? Won't remove (or comment) the if, because it may be needed in a future device.
			#mtd erase $part_label
			#get_image "$1" | mtd -n write - $part_label
			echo "writing \"$1\" UBI image to \"$part_label\" (UBI)..."
			get_image "$1" | mtd write - $part_label
		else
			echo "writing \"$1\" image to \"$part_label\""
			get_image "$1" | mtd write - $part_label
		fi
	}
}
