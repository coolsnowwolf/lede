#!/bin/sh

APM821XX_BOARD_NAME=
APM821XX_MODEL=

apm821xx_board_detect() {
	local model
	local name

	model=$(awk 'BEGIN{FS="[ \t]+:[ \t]"} /model/ {print $2}' /proc/cpuinfo)

	case "$model" in
	*"Meraki MR24 Access Point")
		name="mr24"
		;;

	*"Meraki MX60/MX60W Security Appliance")
		name="mx60"
		;;

	*"MyBook Live"*)
		name="mbl"
		;;

	*"Netgear WNDR4700/WNDR4720 Series")
		name="wndr4700"
		;;

	*)
		name="unknown"
		;;
	esac

	[ -z "$name" ] && name="unknown"

	[ -z "$APM821XX_BOARD_NAME" ] && APM821XX_BOARD_NAME="$name"
	[ -z "$APM821XX_MODEL" ] && APM821XX_MODEL="$model"

	[ -e "/tmp/sysinfo/" ] || mkdir -p "/tmp/sysinfo/"

	echo "$APM821XX_BOARD_NAME" > /tmp/sysinfo/board_name
	echo "$APM821XX_MODEL" > /tmp/sysinfo/model
}
