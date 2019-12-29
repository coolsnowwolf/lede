#
# Copyright (C) 2014-2015 OpenWrt.org
#

linksys_get_target_firmware() {
	cur_boot_part=`/usr/sbin/fw_printenv -n boot_part`
	target_firmware=""
	if [ "$cur_boot_part" = "1" ]
	then
		# current primary boot - update alt boot
		target_firmware="kernel2"
		fw_setenv boot_part 2
		#In EA8500 bootcmd is always "bootipq", so don't change
		#fw_setenv bootcmd "run altnandboot"
	elif [ "$cur_boot_part" = "2" ]
	then
		# current alt boot - update primary boot
		target_firmware="kernel1"
		fw_setenv boot_part 1
		#In EA8500 bootcmd is always "bootipq", so don't change
		#fw_setenv bootcmd "run nandboot"
	fi

	# re-enable recovery so we get back if the new firmware is broken
	fw_setenv auto_recovery yes

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

	if [ ! -n "$part_label" ]
	then
		echo "cannot find target partition"
		exit 1
	fi

	local target_mtd=$(find_mtd_part $part_label)

	[ "$magic_long" = "73797375" ] && {
		CI_KERNPART="$part_label"
		if [ "$part_label" = "kernel1" ]
		then
			CI_UBIPART="rootfs1"
		else
			CI_UBIPART="rootfs2"
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
		# check firmwares' rootfs types
		local oldroot="$(linksys_get_root_magic $target_mtd)"
		local newroot="$(linksys_get_root_magic "$1")"

		if [ "$newroot" = "55424923" -a "$oldroot" = "55424923" ]
		# we're upgrading from a firmware with UBI to one with UBI
		then
			# erase everything to be safe
			mtd erase $part_label
			get_image "$1" | mtd -n write - $part_label
		else
			get_image "$1" | mtd write - $part_label
		fi
	}
}
