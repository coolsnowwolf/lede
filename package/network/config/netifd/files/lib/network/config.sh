#!/bin/sh
# Copyright (C) 2011 OpenWrt.org

. /usr/share/libubox/jshn.sh

find_config() {
	local device="$1"
	local ifdev ifl3dev ifobj
	for ifobj in $(ubus list network.interface.\*); do
		interface="${ifobj##network.interface.}"
		(
			json_load "$(ifstatus $interface)"
			json_get_var ifdev device
			json_get_var ifl3dev l3_device
			if [ "$device" = "$ifdev" ] || [ "$device" = "$ifl3dev" ]; then
				echo "$interface"
				exit 0
			else
				exit 1
			fi
		) && return
	done
}

unbridge() {
	return
}

ubus_call() {
	json_init
	local _data="$(ubus -S call "$1" "$2")"
	[ -z "$_data" ] && return 1
	json_load "$_data"
	return 0
}


fixup_interface() {
	local config="$1"
	local ifname type device l3dev

	config_get type "$config" type
	config_get ifname "$config" ifname
	[ "bridge" = "$type" ] && ifname="br-$config"
	ubus_call "network.interface.$config" status || return 0
	json_get_var l3dev l3_device
	[ -n "$l3dev" ] && ifname="$l3dev"
	json_init
	config_set "$config" ifname "$ifname"
}

scan_interfaces() {
	config_load network
	config_foreach fixup_interface interface
}

prepare_interface_bridge() {
	local config="$1"

	[ -n "$config" ] || return 0
	ubus call network.interface."$config" prepare
}

setup_interface() {
	local iface="$1"
	local config="$2"

	[ -n "$config" ] || return 0
	ubus call network.interface."$config" add_device "{ \"name\": \"$iface\" }"
}

do_sysctl() {
	[ -n "$2" ] && \
		sysctl -n -e -w "$1=$2" >/dev/null || \
		sysctl -n -e "$1"
}
