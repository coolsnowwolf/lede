#!/bin/sh /etc/rc.common
# Copyright (c) 2021 OpenWrt.org

START=50

USE_PROCD=1
PROG=/sbin/ustpd

start_service() {
	procd_open_instance
	procd_set_param command "$PROG"
	procd_set_param respawn
	procd_close_instance
}
