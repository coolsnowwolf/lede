#!/bin/sh /etc/rc.common
# Copyright (C) 2006-2011 OpenWrt.org

START=50

SERVICE_USE_PID=1

start() {
	service_start /usr/bin/srelay -c /etc/srelay.conf -r -s 
}

stop() {
	service_stop /usr/bin/srelay
}
