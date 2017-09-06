#!/bin/sh
# Copyright (C) 2009 OpenWrt.org

setup_switch_dev() {
	local name
	config_get name "$1" name
	name="${name:-$1}"
	[ -d "/sys/class/net/$name" ] && ip link set dev "$name" up
	swconfig dev "$name" load network
}

setup_switch() {
	config_load network
	config_foreach setup_switch_dev switch
}
