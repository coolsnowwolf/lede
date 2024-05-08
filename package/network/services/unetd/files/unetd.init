#!/bin/sh /etc/rc.common
# Copyright (c) 2022 OpenWrt.org

START=19

USE_PROCD=1
PROG=/usr/sbin/unetd

start_service() {
	mkdir -p /var/run/unetd /etc/unetd

	procd_open_instance
	procd_set_param command "$PROG" -h /var/run/unetd/hosts -u /var/run/unetd/socket
	procd_set_param respawn
	procd_set_param limits core="unlimited"
	procd_close_instance
}
