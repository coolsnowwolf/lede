#
# Copyright (C) 2011 OpenWrt.org
#

PART_NAME=firmware

tplink_get_hwid() {
	local part

	part=$(find_mtd_part u-boot)
	[ -z "$part" ] && return 1

	dd if=$part bs=4 count=1 skip=81728 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'
}

tplink_get_image_hwid() {
	get_image "$@" | dd bs=4 count=1 skip=16 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'
}

tplink_get_image_boot_size() {
	get_image "$@" | dd bs=4 count=1 skip=37 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'
}

platform_check_image() {
	local board=$(board_name)
	local magic="$(get_magic_long "$1")"

	[ "$#" -gt 1 ] && return 1

	case $board in
	tl-wdr4900-v1)
		[ "$magic" != "01000000" ] && {
			echo "Invalid image type."
			return 1
		}

		local hwid
		local imageid

		hwid=$(tplink_get_hwid)
		imageid=$(tplink_get_image_hwid "$1")

		[ "$hwid" != "$imageid" ] && {
			echo "Invalid image, hardware ID mismatch, hw:$hwid image:$imageid."
			return 1
		}

		local boot_size

		boot_size=$(tplink_get_image_boot_size "$1")
		[ "$boot_size" != "00000000" ] && {
			echo "Invalid image, it contains a bootloader."
			return 1
		}

		return 0
		;;
	esac

	echo "Sysupgrade is not yet supported on $board."
	return 1
}

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	*)
		default_do_upgrade "$ARGV"
		;;
	esac
}

disable_watchdog() {
	killall watchdog
	( ps | grep -v 'grep' | grep '/dev/watchdog' ) && {
		echo 'Could not disable watchdog'
		return 1
	}
}

append sysupgrade_pre_upgrade disable_watchdog
