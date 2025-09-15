#!/bin/sh
PPP_IPPARAM="$6"

. /lib/netifd/netifd-proto.sh
proto_init_update "$IFNAME" 1 1
proto_set_keep 1
[ -n "$PPP_IPPARAM" ] && {
	[ -n "$LLLOCAL" ] && proto_add_ipv6_address "$LLLOCAL" 128
}
proto_send_update "$PPP_IPPARAM"

[ -d /etc/ppp/ip-up.d ] && {
	for SCRIPT in /etc/ppp/ip-up.d/*
	do
		[ -x "$SCRIPT" ] && "$SCRIPT" "$@"
	done
}

if [ -n "$AUTOIPV6" ]; then
	ZONE=$(fw3 -q network $PPP_IPPARAM 2>/dev/null)

	json_init
	json_add_string name "${PPP_IPPARAM}_6"
	json_add_string ifname "@$PPP_IPPARAM"
	json_add_string proto "dhcpv6"
	[ -n "$ZONE" ] && json_add_string zone "$ZONE"
	[ -n "$EXTENDPREFIX" ] && json_add_string extendprefix 1
	[ -n "$IP6TABLE" ] && json_add_string ip6table $IP6TABLE
	[ -n "$PEERDNS" ] && json_add_boolean peerdns $PEERDNS
	json_close_object
	ubus call network add_dynamic "$(json_dump)"
fi
