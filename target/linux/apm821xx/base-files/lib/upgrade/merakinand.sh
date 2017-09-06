#!/bin/sh
#
# Copyright (C) 2016 Chris Blake <chrisrblake93@gmail.com>
#
# Custom upgrade script for Meraki NAND devices (ex. MR24)
# Based on merakinand.sh from the ar71xx target
#
. /lib/apm821xx.sh
. /lib/functions.sh

merakinand_do_kernel_check() {
	local board_name="$1"
	local tar_file="$2"
	local image_magic_word=`(tar xf $tar_file sysupgrade-$board_name/kernel -O 2>/dev/null | dd bs=1 count=4 skip=0 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"')`

	# What is our kernel magic string?
	case "$board_name" in
	"mr24"|\
	"mx60")
		[ "$image_magic_word" == "8e73ed8a" ] && {
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
	"mr24"|\
	"mx60")
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
	"mr24"|\
	"mx60")
		nand_do_upgrade $1
		;;
	*)
		echo "Unsupported device $board_name";
		exit 1
		;;
	esac
}
