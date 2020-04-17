#!/bin/sh /etc/rc.common

START=00

USE_PROCD=1
NAME=urngd
PROG=/sbin/urngd

start_service() {
	procd_open_instance
	procd_set_param command "$PROG"
	procd_close_instance
}

reload_service() {
	procd_send_signal $PROG
}
