#!/bin/sh
PPP_IPPARAM="$6"

. /lib/netifd/netifd-proto.sh
proto_init_update "$IFNAME" 0
proto_send_update "$PPP_IPPARAM"

[ -d /etc/ppp/ip-down.d ] && {
	for SCRIPT in /etc/ppp/ip-down.d/*
	do
		[ -x "$SCRIPT" ] && "$SCRIPT" "$@"
	done
}
