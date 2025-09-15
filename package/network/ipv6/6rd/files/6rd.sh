#!/bin/sh
# 6rd.sh - IPv6-in-IPv4 tunnel backend
# Copyright (c) 2010-2012 OpenWrt.org

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. /lib/functions/network.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

proto_6rd_setup() {
	local cfg="$1"
	local iface="$2"
	local link="6rd-$cfg"

	local mtu df ttl tos ipaddr peeraddr ip6prefix ip6prefixlen ip4prefixlen tunlink zone
	json_get_vars mtu df ttl tos ipaddr peeraddr ip6prefix ip6prefixlen ip4prefixlen tunlink zone

	[ -z "$ip6prefix" -o -z "$peeraddr" ] && {
		proto_notify_error "$cfg" "MISSING_ADDRESS"
		proto_block_restart "$cfg"
		return
	}

	( proto_add_host_dependency "$cfg" "$peeraddr" "$tunlink" )

	[ -z "$ipaddr" ] && {
		local wanif="$tunlink"
		if [ -z $wanif ] && ! network_find_wan wanif; then
			proto_notify_error "$cfg" "NO_WAN_LINK"
			return
		fi

		if ! network_get_ipaddr ipaddr "$wanif"; then
			proto_notify_error "$cfg" "NO_WAN_LINK"
			return
		fi
	}

	# Determine the relay prefix.
	local ip4prefixlen="${ip4prefixlen:-0}"
	local ip4prefix
	eval "$(ipcalc.sh "$ipaddr/$ip4prefixlen")";ip4prefix=$NETWORK

	# Determine our IPv6 address.
	local ip6subnet=$(6rdcalc "$ip6prefix/$ip6prefixlen" "$ipaddr/$ip4prefixlen")
	local ip6addr="${ip6subnet%%::*}::1"

	# Determine the IPv6 prefix
	local ip6lanprefix="$ip6subnet/$(($ip6prefixlen + 32 - $ip4prefixlen))"

	proto_init_update "$link" 1
	proto_add_ipv6_address "$ip6addr" "$ip6prefixlen"
	proto_add_ipv6_prefix "$ip6lanprefix"

	proto_add_ipv6_route "::" 0 "::$peeraddr" 4096 "" "$ip6addr/$ip6prefixlen"
	proto_add_ipv6_route "::" 0 "::$peeraddr" 4096 "" "$ip6lanprefix"

	proto_add_tunnel
	json_add_string mode sit
	json_add_int mtu "${mtu:-1280}"
	json_add_boolean df "${df:-1}"
	json_add_int ttl "${ttl:-64}"
	[ -n "$tos" ] && json_add_string tos "$tos"
	json_add_string local "$ipaddr"
	[ -n "$tunlink" ] && json_add_string link "$tunlink"

	json_add_object 'data'
	json_add_string prefix "$ip6prefix/$ip6prefixlen"
	json_add_string relay-prefix "$ip4prefix/$ip4prefixlen"
	json_close_object

	proto_close_tunnel

	proto_add_data
	[ -n "$zone" ] && json_add_string zone "$zone"
	proto_close_data

	proto_send_update "$cfg"
}

proto_6rd_teardown() {
	local cfg="$1"
}

proto_6rd_init_config() {
	no_device=1
	available=1

	proto_config_add_int "mtu"
	proto_config_add_boolean "df"
	proto_config_add_int "ttl"
	proto_config_add_string "tos"
	proto_config_add_string "ipaddr"
	proto_config_add_string "peeraddr"
	proto_config_add_string "ip6prefix"
	proto_config_add_string "ip6prefixlen"
	proto_config_add_string "ip4prefixlen"
	proto_config_add_string "tunlink"
	proto_config_add_string "zone"
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol 6rd
}
