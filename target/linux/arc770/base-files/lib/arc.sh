#!/bin/sh
#
# Copyright (C) 2015 OpenWrt.org
#

# defaults
ARC_BOARD_NAME="generic"
ARC_BOARD_MODEL="Generic ARC board"

arc_board_detect() {
	local board
	local model
	local compatible

	[ -e "/tmp/sysinfo/" ] || mkdir -p "/tmp/sysinfo/"

	model="$( cat /proc/device-tree/model )"
	compatible="$( cat /proc/device-tree/compatible )"

	case "$compatible" in
	"snps,axs101""snps,arc-sdp")
		board="arc-sdp";
		;;
	"snps,nsim")
		board="arc-nsim";
		;;
	esac

	if [ "$board" != "" ]; then
		ARC_BOARD_NAME="$board"
	fi

	if [ "$model" != "" ]; then
		ARC_BOARD_MODEL="$model"
	fi

	echo "$ARC_BOARD_NAME" > /tmp/sysinfo/board_name
	echo "$ARC_BOARD_MODEL" > /tmp/sysinfo/model
	echo "Detected $ARC_BOARD_NAME // $ARC_BOARD_MODEL"
}
