#!/bin/sh

ipsets=$(uci -q get dhcp.@dnsmasq[0].ipset)
[ -z "$ipsets" ] && exit 0

for ipset in $ipsets; do
	names=${ipset##*/}
	domains=${ipset%/*}

	[ -z "$names" ] || [ -z "$domains" ] && continue

	uci add dhcp ipset

	OLDIFS="$IFS"

	IFS=","
	for name in $names; do
		uci add_list dhcp.@ipset[-1].name="$name"
	done

	IFS="/"
	for domain in ${domains:1}; do
		uci add_list dhcp.@ipset[-1].domain="$domain"
	done

	IFS="$OLDIFS"

	uci del_list dhcp.@dnsmasq[0].ipset="$ipset"
done

uci commit dhcp
exit 0
