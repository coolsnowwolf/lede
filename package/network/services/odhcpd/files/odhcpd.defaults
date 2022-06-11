#!/bin/sh
uci -q get dhcp.odhcpd && exit 0
touch /etc/config/dhcp

. /usr/share/libubox/jshn.sh

json_load "$(cat /etc/board.json)"
json_select network
json_select lan
json_get_vars protocol
json_select ..
json_select ..

ODHCPDONLY=0
V4MODE=disabled
V6MODE=disabled

[ -e /usr/sbin/dnsmasq ] || ODHCPDONLY=1

case "$protocol" in
# only enable server mode on statically addressed lan ports
"static")
	V4MODE=server
	[ -e /proc/sys/net/ipv6 ] && V6MODE=server
	;;
esac

uci get dhcp.lan 1>/dev/null 2>/dev/null || {
uci batch <<EOF
set dhcp.lan=dhcp
set dhcp.lan.interface='lan'
set dhcp.lan.start='100'
set dhcp.lan.limit='150'
set dhcp.lan.leasetime='12h'
set dhcp.lan.domain='lan'
EOF
}

uci batch <<EOF
set dhcp.odhcpd=odhcpd
set dhcp.odhcpd.maindhcp=$ODHCPDONLY
set dhcp.odhcpd.leasefile=/tmp/hosts/odhcpd
set dhcp.odhcpd.leasetrigger=/usr/sbin/odhcpd-update
set dhcp.odhcpd.loglevel=4
set dhcp.lan.dhcpv4=$V4MODE
set dhcp.lan.dhcpv6=$V6MODE
set dhcp.lan.ra=$V6MODE
set dhcp.lan.ra_slaac=1
add_list dhcp.lan.ra_flags=managed-config
add_list dhcp.lan.ra_flags=other-config
commit dhcp
EOF
