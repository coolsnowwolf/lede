#!/bin/sh /etc/rc.common
# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2013-2020 OpenWrt.org

START=11

boot() {
	/sbin/block mount
}

start() {
	return 0
}

restart() {
	return 0
}

stop() {
	/sbin/block umount
}
