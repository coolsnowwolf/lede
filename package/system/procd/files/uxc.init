#!/bin/sh /etc/rc.common

START=99
USE_PROCD=1
NAME=uxc
PROG=/sbin/uxc

start_service() {
	procd_open_instance "uxc"
	procd_set_param command "$PROG" boot
	procd_close_instance
}

service_triggers() {
	procd_add_raw_trigger "mount.add" 3000 /etc/init.d/uxc start
}
