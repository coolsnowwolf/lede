#!/bin/sh
# gnunet.sh - GNUnet proto for netifd
# Copyright (c) 2016 OpenWrt.org

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. /lib/functions/network.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

proto_gnunet_setup() {
	local cfg="$1"
	local iface="$2"

	local ipv4_addr ipv6_addr prefix ipaddr ip6addr

	local configfile="/var/run/gnunet/gnunet.conf"
	local gnunet_iftype

	for gnit in dns exit vpn; do
		local i1="$(gnunet-config -c $configfile -s $gnit -o IFNAME 2>/dev/null)"
		local i2="$(gnunet-config -c $configfile -s $gnit -o TUN_IFNAME 2>/dev/null)"
		[ "$i1" = "$iface" -o "$i2" = "$iface" ] && gnunet_iftype="$gnit"
	done

	if [ -z "$iface" -o -z "$gnunet_iftype" ]; then
		proto_setup_failed "$cfg"
		return
	fi

	proto_init_update "$iface" 1
	ipaddr="$(gnunet-config -c $configfile -s $gnunet_iftype -o IPV4ADDR 2>/dev/null)"
	ipmask="$(gnunet-config -c $configfile -s $gnunet_iftype -o IPV4MASK 2>/dev/null)"
	ip6addr="$(gnunet-config -c $configfile -s $gnunet_iftype -o IPV6ADDR 2>/dev/null)"
	ip6prefix="$(gnunet-config -c $configfile -s $gnunet_iftype -o IPV6PREFIX 2>/dev/null)"

	[ -n "$ipaddr" ]  && proto_add_ipv4_address "$ipaddr" "$ipmask"
	[ -n "$ip6addr" ] && proto_add_ipv6_address "$ip6addr" "$ip6prefix"
	proto_send_update "$cfg"
}

proto_gnunet_teardown() {
	return
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol gnunet
}


