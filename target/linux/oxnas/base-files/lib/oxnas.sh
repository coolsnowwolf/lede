#!/bin/sh
#
# Copyright (C) 2013 OpenWrt.org
#

OXNAS_BOARD_NAME=
OXNAS_MODEL=

bootloader_cmdline_var() {
	local param
	local pval
	for arg in $(cat /proc/device-tree/chosen/bootloader-args); do
		param="$(echo $arg | cut -d'=' -f 1)"
		pval="$(echo $arg | cut -d'=' -f 2-)"

		if [ "$param" = "$1" ]; then
			echo "$pval"
		fi
	done
}

legacy_boot_mac_adr() {
	local macstr
	local oIFS
	macstr="$(bootloader_cmdline_var mac_adr)"
	oIFS="$IFS"
	IFS=","
	set -- $macstr
	printf "%02x:%02x:%02x:%02x:%02x:%02x" $1 $2 $3 $4 $5 $6
	IFS="$oIFS"
}

oxnas_board_detect() {
	local machine
	local name

	machine=$(cat /proc/device-tree/model)

	case "$machine" in
	*"Akitio MyCloud mini"*)
		name="akitio"
		;;
	*"MitraStar Technology Corp. STG-212"*)
		name="stg212"
		;;
	*"Shuttle KD20"*)
		name="kd20"
		;;
	*"Pogoplug Pro"*)
		name="pogoplug-pro"
		;;
	*"Pogoplug V3"*)
		name="pogoplug-v3"
		;;
	esac

	[ -z "$name" ] && name="unknown"

	[ -z "$OXNAS_BOARD_NAME" ] && OXNAS_BOARD_NAME="$name"
	[ -z "$OXNAS_MODEL" ] && OXNAS_MODEL="$machine"

	[ -e "/tmp/sysinfo/" ] || mkdir -p "/tmp/sysinfo/"

	echo "$OXNAS_BOARD_NAME" > /tmp/sysinfo/board_name
	echo "$OXNAS_MODEL" > /tmp/sysinfo/model
}
