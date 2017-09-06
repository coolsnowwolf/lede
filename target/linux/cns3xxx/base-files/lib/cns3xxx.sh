#!/bin/sh

CNS3XXX_BOARD_NAME=
CNS3XXX_MODEL=

cns3xxx_board_detect() {
	local machine
	local name

	machine=$(awk 'BEGIN{FS="[ \t]+:[ \t]"} /Hardware/ {print $2}' /proc/cpuinfo)

	case "$machine" in
		"Gateworks Corporation Laguna"*)
			name="laguna"
			;;
		*)
			name="generic";
			;;
	esac

	[ -z "$CNS3XXX_BOARD_NAME" ] && CNS3XXX_BOARD_NAME="$name"
	[ -z "$CNS3XXX_MODEL" ] && CNS3XXX_MODEL="$machine"

	[ -e "/tmp/sysinfo/" ] || mkdir -p "/tmp/sysinfo/"

	echo "$CNS3XXX_BOARD_NAME" > /tmp/sysinfo/board_name
	echo "$CNS3XXX_MODEL" > /tmp/sysinfo/model
}
