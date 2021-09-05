#!/bin/sh
# tayga.sh - TAYGA proto
# Copyright (c) 2014 OpenWrt.org

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. /lib/functions/network.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

proto_tayga_setup() {
	local cfg="$1"
	local iface="$2"
	local link="tayga-$cfg"

	local ipv4_addr ipv6_addr prefix dynamic_pool ipaddr ip6addr noroutes
	json_get_vars ipv4_addr ipv6_addr prefix dynamic_pool ipaddr ip6addr noroutes
	[ -z "$ipv4_addr" -o -z "$prefix" ] && {
		proto_notify_error "$cfg" "REQUIRED_PARAMETERS_MISSING"
		proto_block_restart "$cfg"
		return
	}

	local tmpconf="/var/etc/tayga-$cfg.conf"
	mkdir -p /var/etc
	mkdir -p /var/run/tayga/$cfg

	echo "tun-device $link" >$tmpconf
	echo "ipv4-addr $ipv4_addr" >>$tmpconf
	[ -n "$ipv6_addr" ] &&
		echo "ipv6-addr $ipv6_addr" >>$tmpconf
	[ -n "$prefix" ] &&
		echo "prefix $prefix" >>$tmpconf
	[ -n "$dynamic_pool" ] &&
		echo "dynamic-pool $dynamic_pool" >>$tmpconf
	echo "data-dir /var/run/tayga/$cfg" >>$tmpconf
	#TODO: Support static mapping of IPv4 <-> IPv6

	# here we create TUN device and check configuration
	tayga -c $tmpconf --mktun
	[ "$?" -ne 0 ] && {
		proto_notify_error "$cfg" "TAYGA_FAILED"
		proto_block_restart "$cfg"
		return
	}

	proto_init_update "$link" 1

	[ -n "$ipaddr" ]  && proto_add_ipv4_address "$ipaddr" "255.255.255.255"
	[ -n "$ip6addr" ] && proto_add_ipv6_address "$ip6addr" "128"

	[ "$noroutes" != 1 ] && {
		[ -n "$ipv6_addr" ] && proto_add_ipv6_route "$ipv6_addr" "128"
		[ -n "$dynamic_pool" ] && {
			local pool="${dynamic_pool%%/*}"
			local mask="${dynamic_pool##*/}"
			proto_add_ipv4_route "$pool" "$mask"
		}
		[ -n "$prefix" ] && {
			local prefix6="${prefix%%/*}"
			local mask6="${prefix##*/}"
			proto_add_ipv6_route "$prefix6" "$mask6"
		}
	}

	proto_send_update "$cfg"

	proto_run_command "$cfg" tayga -n -c $tmpconf \
		-p /var/run/$link.pid

}

proto_tayga_teardown() {
	local cfg="$1"
	local tmpconf="/var/etc/tayga-$cfg.conf"
	proto_kill_command "$cfg"
	sleep 1
	tayga -c $tmpconf --rmtun
}

proto_tayga_init_config() {
	no_device=1
	available=1
	proto_config_add_string "ipv4_addr"
	proto_config_add_string "ipv6_addr"
	proto_config_add_string "prefix"
	proto_config_add_string "dynamic_pool"
	proto_config_add_string "ipaddr"
	proto_config_add_string "ip6addr:ip6addr"
	proto_config_add_boolean "noroutes"
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol tayga
}
