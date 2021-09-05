#!/bin/sh /etc/rc.common
# Copyright (C) 2006-2011 OpenWrt.org

START=50

start() {
	mkdir -m 0755 -p /var/run/vsftpd
	service_start /usr/sbin/vsftpd
}

stop() {
	service_stop /usr/sbin/vsftpd
}
