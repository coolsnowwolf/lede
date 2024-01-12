#!/bin/sh /etc/rc.common
# Copyright (c) 2014 OpenWrt.org

START=80

USE_PROCD=1
PROG=/usr/sbin/ugps

service_triggers() {
	procd_add_reload_trigger gps
}

start_service() {
	local tty="$(uci get gps.@gps[-1].tty)"
	local atime="$(uci get gps.@gps[-1].adjust_time)"
	local disabled="$(uci get gps.@gps[-1].disabled || echo 0)"

	[ "$disabled" == "0" ] || return
	[ "$tty" ] || return

	case "$tty" in
		"/"*)
			true
			;;
		*)
			tty="/dev/$tty"
			;;
	esac

	procd_open_instance
	procd_set_param command "$PROG"
	[ "$atime" -eq 0 ] || procd_append_param command "-a"
	procd_append_param command "$tty"
	procd_set_param respawn
	procd_close_instance
}
