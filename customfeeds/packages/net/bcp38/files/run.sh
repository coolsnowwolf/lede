#!/bin/sh
# BCP38 filtering implementation for CeroWrt.
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 3 of the License, or (at your option) any later
# version.
#
# Author: Toke Høiland-Jørgensen <toke@toke.dk>

STOP=$1
IPSET_NAME=bcp38-ipv4
IPTABLES_CHAIN=BCP38

. /lib/functions.sh

config_load bcp38

add_bcp38_rule()
{
	local subnet="$1"
	local action="$2"

	if [ "$action" == "nomatch" ]; then
		ipset add "$IPSET_NAME" "$subnet" nomatch
	else
		ipset add "$IPSET_NAME" "$subnet"
	fi
}

detect_upstream()
{
	local interface="$1"

	subnets=$(ip route show dev "$interface"  | grep 'scope link' | awk '{print $1}')
	for subnet in $subnets; do
		# ipset test doesn't work for subnets, so strip out the subnet part
		# and test for that; add as exception if there's a match
		addr=$(echo $subnet | sed 's|/[0-9]\+$||')
		ipset test "$IPSET_NAME" $addr 2>/dev/null && add_bcp38_rule $subnet nomatch
	done
}

run() {
    	local section="$1"
    	local enabled
	local interface
	local detect_upstream
	config_get_bool enabled "$section" enabled 0
	config_get interface "$section" interface
	config_get detect_upstream "$section" detect_upstream

	if [ "$enabled" -eq "1" -a -n "$interface" -a -z "$STOP" ] ; then
		setup_ipset
		setup_iptables "$interface"
		config_list_foreach "$section" match add_bcp38_rule match
		config_list_foreach "$section" nomatch add_bcp38_rule nomatch
		[ "$detect_upstream" -eq "1" ] && detect_upstream "$interface"
	fi
	exit 0
}

setup_ipset()
{
	ipset create "$IPSET_NAME" hash:net family ipv4
	ipset flush "$IPSET_NAME"
}

setup_iptables()
{
	local interface="$1"
	iptables -N "$IPTABLES_CHAIN" 2>/dev/null
	iptables -F "$IPTABLES_CHAIN" 2>/dev/null

	iptables -I output_rule -m conntrack --ctstate NEW -j "$IPTABLES_CHAIN"
	iptables -I input_rule -m conntrack --ctstate NEW -j "$IPTABLES_CHAIN"
	iptables -I forwarding_rule -m conntrack --ctstate NEW -j "$IPTABLES_CHAIN"

	# always accept DHCP traffic
	iptables -A "$IPTABLES_CHAIN" -p udp --dport 67:68 --sport 67:68 -j RETURN
	iptables -A "$IPTABLES_CHAIN" -o "$interface" -m set --match-set "$IPSET_NAME" dst -j REJECT --reject-with icmp-net-unreachable
	iptables -A "$IPTABLES_CHAIN" -i "$interface" -m set --match-set "$IPSET_NAME" src -j DROP
}

destroy_ipset()
{
	ipset flush "$IPSET_NAME" 2>/dev/null
	ipset destroy "$IPSET_NAME" 2>/dev/null
}

destroy_iptables()
{
	iptables -D output_rule -m conntrack --ctstate NEW -j "$IPTABLES_CHAIN" 2>/dev/null
	iptables -D input_rule -m conntrack --ctstate NEW -j "$IPTABLES_CHAIN" 2>/dev/null
	iptables -D forwarding_rule -m conntrack --ctstate NEW -j "$IPTABLES_CHAIN" 2>/dev/null
	iptables -F "$IPTABLES_CHAIN" 2>/dev/null
	iptables -X "$IPTABLES_CHAIN" 2>/dev/null
}

destroy_iptables
destroy_ipset
config_foreach run bcp38

exit 0
