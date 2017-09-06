#!/bin/sh
# Copyright (C) 2010 OpenWrt.org

set_led() {
	local state="$1"
	[ -f "/proc/adm8668/sesled" ] && echo "$state" > "/proc/adm8668/sesled"
}

set_state() {
	case "$1" in
		preinit)
			set_led 1
		;;
		failsafe)
			set_led 2
		;;
		done)
			set_led 0
		;;
	esac
}
