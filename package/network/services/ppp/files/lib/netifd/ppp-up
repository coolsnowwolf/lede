#!/bin/sh
PPP_IPPARAM="$6"

. /lib/netifd/netifd-proto.sh
proto_init_update "$IFNAME" 1 1
proto_set_keep 1
[ -n "$PPP_IPPARAM" ] && {
	[ -n "$IPLOCAL" ] && proto_add_ipv4_address "$IPLOCAL" 32 "" "${IPREMOTE:-2.2.2.2}"
	[ -n "$IPREMOTE" ] && proto_add_ipv4_route 0.0.0.0 0 "$IPREMOTE"
	[ -n "$DNS1" ] && proto_add_dns_server "$DNS1"
	[ -n "$DNS2" -a "$DNS1" != "$DNS2" ] && proto_add_dns_server "$DNS2"
}
proto_send_update "$PPP_IPPARAM"

[ -d /etc/ppp/ip-up.d ] && {
	for SCRIPT in /etc/ppp/ip-up.d/*
	do
		[ -x "$SCRIPT" ] && "$SCRIPT" "$@"
	done
}
