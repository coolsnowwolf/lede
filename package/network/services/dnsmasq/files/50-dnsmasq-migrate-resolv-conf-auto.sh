#!/bin/sh

[ "$(uci get dhcp.@dnsmasq[0].resolvfile)" = "/tmp/resolv.conf.auto" ] && {
	uci set dhcp.@dnsmasq[0].resolvfile="/tmp/resolv.conf.d/resolv.conf.auto"
	uci commit dhcp
}

exit 0
