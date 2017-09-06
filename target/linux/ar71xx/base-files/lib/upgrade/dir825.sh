#!/bin/sh
#
# Copyright (C) 2012 OpenWrt.org
#

. /lib/functions.sh
. /lib/ar71xx.sh

get_magic_at() {
	local mtddev=$1
	local pos=$2
	dd bs=1 count=2 skip=$pos if=$mtddev 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'
}

dir825b_is_caldata_valid() {
	local mtddev=$1
	local magic

	magic=$(get_magic_at $mtddev 4096)
	[ "$magic" != "a55a" ] && return 0

	magic=$(get_magic_at $mtddev 20480)
	[ "$magic" != "a55a" ] && return 0

	return 1
}

dir825b_copy_caldata() {
	local cal_src=$1
	local cal_dst=$2
	local mtd_src
	local mtd_dst
	local md5_src
	local md5_dst

	mtd_src=$(find_mtd_part $cal_src)
	[ -z "$mtd_src" ] && {
		echo "no $cal_src partition found"
		return 1
	}

	mtd_dst=$(find_mtd_part $cal_dst)
	[ -z "$mtd_dst" ] && {
		echo "no $cal_dst partition found"
		return 1
	}

	dir825b_is_caldata_valid "$mtd_src" && {
		echo "no valid calibration data found in $cal_src"
		return 1
	}

	dir825b_is_caldata_valid "$mtd_dst" && {
		echo "Copying calibration data from $cal_src to $cal_dst..."
		dd if="$mtd_src" 2>/dev/null | mtd -q -q write - "$cal_dst"
	}

        md5_src=$(md5sum "$mtd_src") && md5_src="${md5_src%% *}"
        md5_dst=$(md5sum "$mtd_dst") && md5_dst="${md5_dst%% *}"

	[ "$md5_src" != "$md5_dst" ] && {
		echo "calibration data mismatch $cal_src:$md5_src $cal_dst:$md5_dst"
		return 1
	}

	return 0
}

dir825b_do_upgrade_combined() {
	local fw_part=$1
	local fw_file=$2
	local fw_mtd=$(find_mtd_part $fw_part)
	local fw_length=0x$(dd if="$fw_file" bs=2 skip=1 count=4 2>/dev/null)
	local fw_blocks=$(($fw_length / 65536))

	if [ -n "$fw_mtd" ] &&  [ ${fw_blocks:-0} -gt 0 ]; then
		local append=""
		[ -f "$CONF_TAR" -a "$SAVE_CONFIG" -eq 1 ] && append="-j $CONF_TAR"

		sync
		dd if="$fw_file" bs=64k skip=1 count=$fw_blocks 2>/dev/null | \
			mtd $append write - "$fw_part"
	fi
}

dir825b_check_image() {
	local magic="$(get_magic_long "$1")"
	local fw_mtd=$(find_mtd_part "firmware_orig")

	case "$magic" in
	"27051956")
		;;
	"43493030")
		local md5_img=$(dd if="$1" bs=2 skip=9 count=16 2>/dev/null)
		local md5_chk=$(dd if="$1" bs=64k skip=1 2>/dev/null | md5sum -); md5_chk="${md5_chk%% *}"
		local fw_len=$(dd if="$1" bs=2 skip=1 count=4 2>/dev/null)
		local fw_part_len=$(mtd_get_part_size "firmware")

		if [ -z "$fw_mtd" ]; then
			ask_bool 0 "Do you have a backup of the caldata partition?" || {
				echo "Warning, please make sure that you have a backup of the caldata partition."
				echo "Once you have that, use 'sysupgrade -i' for upgrading to the 'fat' firmware."
				return 1
			}
		fi

		if [ -z "$md5_img" -o -z "$md5_chk" ]; then
			echo "Unable to get image checksums. Maybe you are using a streamed image?"
			return 1
		fi

		if [ "$md5_img" != "$md5_chk" ]; then
			echo "Invalid image. Contents do not match checksum (image:$md5_img calculated:$md5_chk)"
			return 1
		fi

		fw_len=$((0x$fw_len))
		fw_part_len=${fw_part_len:-0}

		if [ $fw_part_len -lt $fw_len ]; then
			echo "The upgrade image is too big (size:$fw_len available:$fw_part_len)"
			return 1
		fi
		;;
	*)
		echo "Unsupported image format."
		return 1
		;;
	esac

	return 0
}

platform_do_upgrade_dir825b() {
	local magic="$(get_magic_long "$1")"
	local fw_mtd=$(find_mtd_part "firmware_orig")

	case "$magic" in
	"27051956")
		if [ -n "$fw_mtd" ]; then
			# restore calibration data before downgrading to
			# the normal image
			dir825b_copy_caldata "caldata" "caldata_orig" || {
				echo "unable to restore calibration data"
				exit 1
			}
			PART_NAME="firmware_orig"
		else
			PART_NAME="firmware"
		fi
		default_do_upgrade "$ARGV"
		;;
	"43493030")
		if [ -z "$fw_mtd" ]; then
			# backup calibration data before upgrading to the
			# fat image
			dir825b_copy_caldata "caldata" "caldata_copy" || {
				echo "unable to backup calibration data"
				exit 1
			}
		fi
		dir825b_do_upgrade_combined "firmware" "$ARGV"
		;;
	esac
}
