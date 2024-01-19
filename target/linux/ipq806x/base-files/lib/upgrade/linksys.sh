#
# Copyright (C) 2014-2015 OpenWrt.org
#

linksys_get_target_firmware() {

	local cur_boot_part mtd_ubi0

	cur_boot_part=$(/usr/sbin/fw_printenv -n boot_part)
	if [ -z "${cur_boot_part}" ] ; then
		mtd_ubi0=$(cat /sys/devices/virtual/ubi/ubi0/mtd_num)
		case $(grep -E ^mtd${mtd_ubi0}: /proc/mtd | cut -d '"' -f 2) in
		kernel1|rootfs1)
			cur_boot_part=1
			;;
		kernel2|rootfs2)
			cur_boot_part=2
			;;
		esac
		>&2 printf "Current boot_part='%s' selected from ubi0/mtd_num='%s'" \
			"${cur_boot_part}" "${mtd_ubi0}"
	fi

	case $cur_boot_part in
	1)
		fw_setenv -s - <<-EOF
			boot_part 2
			auto_recovery yes
		EOF
		printf "kernel2"
		return
		;;
	2)
		fw_setenv -s - <<-EOF
			boot_part 1
			auto_recovery yes
		EOF
		printf "kernel1"
		return
		;;
	*)
		return
		;;
	esac
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
		get_image "$1" | mtd write - $part_label
	}
}
