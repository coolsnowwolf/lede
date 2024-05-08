#!/bin/sh /etc/rc.common
# Copyright (c) 2022 OpenWrt.org

START=19

USE_PROCD=1
PROG=/usr/sbin/unet-dht

unet_dht_id() {
	cat \
		/sys/class/net/eth?/address \
		/sys/class/ieee80211/phy*/macaddress \
		/etc/board.json | md5sum | awk '{ print $1 }'
}

start_service() {
	mkdir -p /var/run/unetd /etc/unetd

	procd_open_instance
	procd_set_param command "$PROG" -u /var/run/unetd/socket -n /var/run/unetd/nodes.dat $(unet_dht_id)
	procd_set_param respawn
	procd_set_param limits core="unlimited"
	procd_close_instance
}
