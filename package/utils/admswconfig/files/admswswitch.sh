#!/bin/sh
# Copyright (C) 2006 OpenWrt.org

setup_switch_vlan() {
	config_get ports "$CONFIG_SECTION" "eth$1"
	ports=`echo "$ports"| sed s/" "/""/g`
	ip link set dev eth$1 down
	admswconfig eth$1 ${ports}c
	ip link set dev eth$1 up
}

setup_switch() {
	config_cb() {
		case "$1" in
			switch)
				option_cb() {
					case "$1" in
						eth*) setup_switch_vlan "${1##eth}";;
					esac
				}
			;;
			*)
				option_cb() { return 0; }
			;;
		esac
	}
	config_load network
}
