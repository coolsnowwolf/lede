#!/bin/sh
#
# Copyright (C) 2013 OpenWrt.org
#

MPC85XX_BOARD_NAME=
MPC85XX_MODEL=

mpc85xx_board_detect() {
	local model
	local name

	model=$(awk 'BEGIN{FS="[ \t]+:[ \t]"} /model/ {print $2}' /proc/cpuinfo)

	case "$model" in
	*"TL-WDR4900 v1")
		name="tl-wdr4900-v1"
		;;
	esac

	[ -z "$name" ] && name="unknown"

	[ -z "$MPC85XX_BOARD_NAME" ] && MPC85XX_BOARD_NAME="$name"
	[ -z "$MPC85XX_MODEL" ] && MPC85XX_MODEL="$model"

	[ -e "/tmp/sysinfo/" ] || mkdir -p "/tmp/sysinfo/"

	echo "$MPC85XX_BOARD_NAME" > /tmp/sysinfo/board_name
	echo "$MPC85XX_MODEL" > /tmp/sysinfo/model
}
