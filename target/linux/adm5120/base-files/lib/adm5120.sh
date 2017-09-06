#!/bin/sh
#
# Copyright (C) 2007 OpenWrt.org
#
#

board_name=""
status_led=""
sys_mtd_part=""

adm5120_detect() {
	board_name=$(awk 'BEGIN{FS="[ \t]+:[ \t]"} /machine/ {print $2}' /proc/cpuinfo)

	case "$board_name" in
	"Cellvision"*)
		status_led="status"
		sys_mtd_part="firmware"
		;;
	"Compex"*)
		status_led="diag"
		case "$board_name" in
		*-WRT)
			sys_mtd_part="trx"
			;;
		*)
			sys_mtd_part="partition1"
			;;
		esac
		;;
	"Edimax"*)
		status_led="power"
		sys_mtd_part="firmware"
		;;
	"Infineon"*)
		sys_mtd_part="firmware"
		;;
	"Mikrotik"*)
		status_led="power"
		;;
	"ZyXEL"*)
		status_led="power"
		sys_mtd_part="trx"
		;;
	"EB-214A"*)
		status_led="power"
		sys_mtd_part="firmware"
		;;
	*)
		;;
	esac
}

adm5120_detect
