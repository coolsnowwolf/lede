#!/bin/sh
#
# Copyright (C) 2012 OpenWrt.org
#

IXP4XX_BOARD_NAME=
IXP4XX_MODEL=

ixp4xx_board_detect() {
	local machine
	local name

	machine=$(awk 'BEGIN{FS="[ \t]+:[ \t]"} /Hardware/ {print $2}' /proc/cpuinfo)

	case "$machine" in
	"Gateworks Cambria"*)
		name="cambria"
		;;
	"Gateworks Avila"*)
		name="avila"
		;;
	*)
		name="generic";
		;;
	esac

	[ -z "$IXP4XX_BOARD_NAME" ] && IXP4XX_BOARD_NAME="$name"
	[ -z "$IXP4XX_MODEL" ] && IXP4XX_MODEL="$machine"

	[ -e "/tmp/sysinfo/" ] || mkdir -p "/tmp/sysinfo/"

	echo "$IXP4XX_BOARD_NAME" > /tmp/sysinfo/board_name
	echo "$IXP4XX_MODEL" > /tmp/sysinfo/model
}
