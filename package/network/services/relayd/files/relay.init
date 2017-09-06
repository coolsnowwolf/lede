#!/bin/sh /etc/rc.common
# Copyright (c) 2011-2012 OpenWrt.org

START=80

USE_PROCD=1
PROG=/usr/sbin/relayd

validate_proto_relayd()
{
	uci_validate_section network "interface" "${1}" \
		'network:list(string)' \
		'expiry:uinteger:30' \
		'retry:uinteger:5' \
		'table:range(0, 65535):16800' \
		'forward_bcast:bool:1' \
		'forward_dhcp:bool:1'
}

resolve_ifname() {
	grep -qs "^ *$1:" /proc/net/dev && {
		append resolved_ifnames "$1"
	}
}

resolve_network() {
	local ifn
	fixup_interface "$1"
	config_get ifn "$1" ifname
	[ -z "$ifn" ] && return 1
	resolve_ifname "$ifn"
}

start_relay() {
	local cfg="$1"
	local proto disabled

	config_get proto "$cfg" proto
	[ "$proto" = "relay" ] || return 0

	config_get_bool disabled "$cfg" disabled 0
	[ "$disabled" -gt 0 ] && return 0

	local resolved_ifnames
	local net networks
	config_get networks "$cfg" network
	for net in $networks; do
		resolve_network "$net" || {
			return 1
		}
	done

	local ifn ifnames
	config_get ifnames "$cfg" ifname
	for ifn in $ifnames; do
		resolve_ifname "$ifn" || {
			return 1
		}
	done

	procd_open_instance
	procd_set_param command "$PROG"

	for ifn in $resolved_ifnames; do
		procd_append_param command -I "$ifn"
		procd_append_param netdev "$ifn"
	done
	local ipaddr
	config_get ipaddr "$cfg" ipaddr
	[ -n "$ipaddr" ] && procd_append_param command -L "$ipaddr"

	local gateway
	config_get gateway "$cfg" gateway
	[ -n "$gateway" ] && procd_append_param command -G "$gateway"

	local expiry # = 30
	config_get expiry "$cfg" expiry
	[ -n "$expiry" ] && procd_append_param command -t "$expiry"

	local retry # = 5
	config_get retry "$cfg" retry
	[ -n "$retry" ] && procd_append_param command -p "$retry"

	local table # = 16800
	config_get table "$cfg" table
	[ -n "$table" ] && procd_append_param command -T "$table"

	local fwd_bcast # = 1
	config_get_bool fwd_bcast "$cfg" forward_bcast 1
	[ $fwd_bcast -eq 1 ] && procd_append_param command "-B"

	local fwd_dhcp # = 1
	config_get_bool fwd_dhcp "$cfg" forward_dhcp 1
	[ $fwd_dhcp -eq 1 ] && procd_append_param command "-D"

	procd_close_instance
}

service_triggers()
{
	procd_add_reload_trigger "network"
	procd_add_raw_trigger "interface.*" 2000 /etc/init.d/relayd reload
}

start_service() {
	include /lib/network
	config_load network
	config_foreach start_relay interface
}
