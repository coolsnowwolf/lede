#!/bin/sh
# Copyright (C) 2015-2016 OpenWrt.org
# Copyright (C) 2017 LEDE project

ifname=""

brcm2708_detect() {
	local board_name model

	model=$(cat /proc/device-tree/model)
	case "$model" in
	"Raspberry Pi 2 Model B Rev"*)
		board_name="rpi-2-b"
		;;
	"Raspberry Pi 3 Model B Rev"*)
		board_name="rpi-3-b"
		;;
	"Raspberry Pi Compute Module Rev"*)
		board_name="rpi-cm"
		;;
	"Raspberry Pi Model B Plus Rev"* |\
	"Raspberry Pi Model B+ Rev"*)
		board_name="rpi-b-plus"
		;;
	"Raspberry Pi Model B Rev"*)
		board_name="rpi-b"
		;;
	"Raspberry Pi Zero Rev"*)
		board_name="rpi-zero"
		;;
	"Raspberry Pi Zero W Rev"*)
		board_name="rpi-zero-w"
		;;
	*)
		board_name="unknown"
		;;
	esac

	[ -e "/tmp/sysinfo" ] || mkdir -p "/tmp/sysinfo"

	echo "$board_name" > /tmp/sysinfo/board_name
	echo "$model" > /tmp/sysinfo/model
}
