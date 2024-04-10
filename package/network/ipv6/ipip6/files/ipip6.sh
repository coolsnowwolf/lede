#!/bin/sh
# ipip6.sh - ipip6 tunnel backend
# Copyright (c) 2013 OpenWrt.org

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. /lib/functions/network.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

proto_ipip6_setup() {
	local cfg="$1"
	local iface="$2"
	local link="ipip6-$cfg"
	local remoteip6

	local mtu ttl peeraddr ip6addr tunlink zone weakif encaplimit ip4ifaddr
	json_get_vars mtu ttl peeraddr ip6addr tunlink zone weakif encaplimit ip4ifaddr

	[ -z "$peeraddr" ] && {
		proto_notify_error "$cfg" "MISSING_ADDRESS"
		proto_block_restart "$cfg"
		return
	}

	( proto_add_host_dependency "$cfg" "::" "$tunlink" )

	remoteip6=$(resolveip -6 "$peeraddr")
	if [ -z "$remoteip6" ]; then
		sleep 3
		remoteip6=$(resolveip -6 "$peeraddr")
		if [ -z "$remoteip6" ]; then
			proto_notify_error "$cfg" "AFTR_DNS_FAIL"
			return
		fi
	fi

	for ip6 in $remoteip6; do
		peeraddr=$ip6
		break
	done

	[ -z "$ip6addr" ] && {
		local wanif="$tunlink"
		if [ -z "$wanif" ] && ! network_find_wan6 wanif; then
			proto_notify_error "$cfg" "NO_WAN_LINK"
			return
		fi

		if ! network_get_ipaddr6 ip6addr "$wanif"; then
			[ -z "$weakif" ] && weakif="lan"
			if ! network_get_ipaddr6 ip6addr "$weakif"; then
				proto_notify_error "$cfg" "NO_WAN_LINK"
				return
			fi
		fi
	}

	proto_init_update "$link" 1
	proto_add_ipv4_route "0.0.0.0" 0
	proto_add_ipv4_address "$ip4ifaddr" "" "" "0.0.0.0"

	proto_add_tunnel
	json_add_string mode ipip6
	json_add_int mtu "${mtu:-1280}"
	json_add_int ttl "${ttl:-64}"
	json_add_string local "$ip6addr"
	json_add_string remote "$peeraddr"
	[ -n "$tunlink" ] && json_add_string link "$tunlink"
	json_add_object "data"
	  [ -n "$encaplimit" ] && json_add_string encaplimit "$encaplimit"
	json_close_object
	proto_close_tunnel

	proto_add_data
	[ -n "$zone" ] && json_add_string zone "$zone"

	proto_close_data

	proto_send_update "$cfg"
}

proto_ipip6_teardown() {
	local cfg="$1"
}

proto_ipip6_init_config() {
	no_device=1
	available=1

	proto_config_add_string "ip6addr"
	proto_config_add_string "peeraddr"
	proto_config_add_string "tunlink"
	proto_config_add_int "mtu"
	proto_config_add_int "ttl"
	proto_config_add_string "encaplimit"
	proto_config_add_string "zone"
	proto_config_add_string "weakif"
	proto_config_add_string "ip4ifaddr"
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol ipip6
}
