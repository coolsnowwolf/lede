#!/bin/sh
#
# Copyright (C) 2012 OpenWrt.org
#

MCS814X_BOARD_NAME=
MCS814X_MODEL=

mcs814x_board_detect() {
	local machine
	local name

	machine=$(cat /proc/device-tree/model)

	case "$machine" in
	*"Devolo dLAN USB Extender")
		name="dlan-usb-extender"
		;;
	*"Tigal RBT-832")
		name="rbt-832"
		;;
	esac

	[ -z "$name" ] && name="unknown"

	[ -z "$MCS814X_BOARD_NAME" ] && MCS814X_BOARD_NAME="$name"
	[ -z "$MCS814X_MODEL" ] && MCS814X_MODEL="$machine"

	[ -e "/tmp/sysinfo/" ] || mkdir -p "/tmp/sysinfo/"

	echo "$MCS814X_BOARD_NAME" > /tmp/sysinfo/board_name
	echo "$MCS814X_MODEL" > /tmp/sysinfo/model
}
