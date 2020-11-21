#!/bin/sh /etc/rc.common

START=12

USE_PROCD=1
NAME=rpcd
PROG=/sbin/rpcd

start_service() {
	local socket=$(uci -q get rpcd.@rpcd[0].socket)
	local timeout=$(uci -q get rpcd.@rpcd[0].timeout)

	procd_open_instance
	procd_set_param command "$PROG" ${socket:+-s "$socket"} ${timeout:+-t "$timeout"}
	procd_set_param respawn
	procd_close_instance
}

reload_service() {
	procd_send_signal rpcd
}
