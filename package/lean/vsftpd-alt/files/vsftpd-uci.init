#!/bin/sh /etc/rc.common
# Copyright (C) 2006-2011 OpenWrt.org

START=50

start() {
	/usr/sbin/vsftpd_prepare
	service_start /usr/sbin/vsftpd_wrapper ipv4
	service_start /usr/sbin/vsftpd_wrapper ipv6
}

stop() {
	service_stop /usr/sbin/vsftpd
}
