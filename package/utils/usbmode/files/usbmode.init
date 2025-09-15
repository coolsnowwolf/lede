#!/bin/sh /etc/rc.common
# Copyright (C) 2013 OpenWrt.org

START=20
USE_PROCD=1

start_service()
{
	procd_open_instance
	procd_set_param command "/sbin/usbmode" -s
	procd_close_instance
}
