#!/bin/sh /etc/rc.common

START=18

start() {
	[ ! -f /etc/config/wireless ] && {
		/sbin/wifi config
	}
}
