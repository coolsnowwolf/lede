#!/bin/sh
#
# Copyright (C) 2015-2016 Chris Blake <chrisrblake93@gmail.com>
#
# Custom upgrade script for Meraki NAND devices (ex. MR18)
# Based on dir825.sh and stock nand functions
#
. /lib/ar71xx.sh
. /lib/functions.sh

get_magic_at() {
	local mtddev=$1
	local pos=$2
	dd bs=1 count=2 skip=$pos if=$mtddev 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'
}

meraki_is_caldata_valid() {
	local board=$1
	local mtddev=$2
	local magic

	case "$board" in
	"mr18")
		magic=$(get_magic_at $mtddev 4096)
		[ "$magic" != "0202" ] && return 0

		magic=$(get_magic_at $mtddev 20480)
		[ "$magic" != "0202" ] && return 0

		magic=$(get_magic_at $mtddev 36864)
		[ "$magic" != "0202" ] && return 0

		return 1
		;;
	"z1")
		magic=$(get_magic_at $mtddev 4096)
		[ "$magic" != "0202" ] && return 0

		magic=$(get_magic_at $mtddev 86016)
		[ "$magic" != "a55a" ] && return 0

		return 1
		;;
	*)
		return 1
		;;
	esac
}

merakinand_copy_caldata() {
	local cal_src=$1
	local cal_dst=$2
	local ubidev="$(nand_find_ubi $CI_UBIPART)"
	local board_name="$(board_name)"
	local rootfs_size="$(ubinfo /dev/ubi0 -N rootfs_data | grep "Size" | awk '{ print $6 }')"

	# Setup partitions using board name, in case of future platforms
	case "$board_name" in
	"mr18"|\
	"z1")
		# Src is MTD
		mtd_src="$(find_mtd_chardev $cal_src)"
		[ -n "$mtd_src" ] || {
			echo "no mtd device found for partition $cal_src"
			exit 1
		}

		# Dest is UBI
		# TODO: possibly add create (hard to do when rootfs_data is expanded & mounted)
		# Would need to be done from ramdisk
		mtd_dst="$(nand_find_volume $ubidev $cal_dst)"
		[ -n "$mtd_dst" ] || {
			echo "no ubi device found for partition $cal_dst"
			exit 1
		}

		meraki_is_caldata_valid "$board_name" "$mtd_src" && {
			echo "no valid calibration data found in $cal_src"
			exit 1
		}

		meraki_is_caldata_valid "$board_name" "/dev/$mtd_dst" && {
			echo "Copying calibration data from $cal_src to $cal_dst..."
			dd if="$mtd_src" of=/tmp/caldata.tmp 2>/dev/null
			ubiupdatevol "/dev/$mtd_dst" /tmp/caldata.tmp
			rm /tmp/caldata.tmp
			sync
		}
		return 0
		;;
	*)
		echo "Unsupported device $board_name";
		return 1
		;;
	esac
}

merakinand_do_kernel_check() {
	local board_name="$1"
	local tar_file="$2"
	local image_magic_word=`(tar xf $tar_file sysupgrade-$board_name/kernel -O 2>/dev/null | dd bs=1 count=4 skip=0 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"')`

	# What is our kernel magic string?
	case "$board_name" in
	"mr18")
		[ "$image_magic_word" == "8e73ed8a" ] && {
			echo "pass" && return 0
		}
		;;
	"z1")
		[ "$image_magic_word" == "4d495053" ] && {
			echo "pass" && return 0
		}
		;;
	esac

	exit 1
}

merakinand_do_platform_check() {
	local board_name="$1"
	local tar_file="$2"
	local control_length=`(tar xf $tar_file sysupgrade-$board_name/CONTROL -O | wc -c) 2> /dev/null`
	local file_type="$(identify_tar $2 sysupgrade-$board_name/root)"
	local kernel_magic="$(merakinand_do_kernel_check $1 $2)"

	case "$board_name" in
	"mr18"|\
	"z1")
		[ "$control_length" = 0 -o "$file_type" != "squashfs" -o "$kernel_magic" != "pass" ] && {
			echo "Invalid sysupgrade file for $board_name"
			return 1
		}
		;;
	*)
		echo "Unsupported device $board_name";
		return 1
		;;
	esac

	return 0
}

merakinand_do_upgrade() {
	local tar_file="$1"
	local board_name="$(board_name)"

	# Do we need to do any platform tweaks?
	case "$board_name" in
	"mr18")
		# Check and create UBI caldata if it's invalid
		merakinand_copy_caldata "odm-caldata" "caldata"
		nand_do_upgrade $1
		;;
	"z1")
		# Check and create UBI caldata if it's invalid
		merakinand_copy_caldata "origcaldata" "caldata"
		nand_do_upgrade $1
		;;
	*)
		echo "Unsupported device $board_name";
		exit 1
		;;
	esac
}
