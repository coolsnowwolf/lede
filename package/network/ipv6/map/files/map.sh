#!/bin/sh
# map.sh - IPv4-in-IPv6 tunnel backend
#
# Author: Steven Barth <cyrus@openwrt.org>
# Copyright (c) 2014 cisco Systems, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2
# as published by the Free Software Foundation
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. /lib/functions/network.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

proto_map_setup() {
	local cfg="$1"
	local iface="$2"
	local link="map-$cfg"

	# uncomment for legacy MAP0 mode
	#export LEGACY=1

	local type mtu ttl tunlink zone encaplimit
	local rule ipaddr ip4prefixlen ip6prefix ip6prefixlen peeraddr ealen psidlen psid offset
	json_get_vars type mtu ttl tunlink zone encaplimit
	json_get_vars rule ipaddr ip4prefixlen ip6prefix ip6prefixlen peeraddr ealen psidlen psid offset

	[ "$zone" = "-" ] && zone=""
	[ -z "$type" ] && type="map-e"
	[ -z "$ip4prefixlen" ] && ip4prefixlen=32

	( proto_add_host_dependency "$cfg" "::" "$tunlink" )

	# fixme: handle RA/DHCPv6 address race for LW
	[ "$type" = lw4o6 ] && sleep 5

	if [ -z "$rule" ]; then
		rule="type=$type,ipv6prefix=$ip6prefix,prefix6len=$ip6prefixlen,ipv4prefix=$ipaddr,prefix4len=$ip4prefixlen"
		[ -n "$psid" ] && rule="$rule,psid=$psid"
		[ -n "$psidlen" ] && rule="$rule,psidlen=$psidlen"
		[ -n "$offset" ] && rule="$rule,offset=$offset"
		[ -n "$ealen" ] && rule="$rule,ealen=$ealen"
		if [ "$type" = "map-t" ]; then
			rule="$rule,dmr=$peeraddr"
		else
			rule="$rule,br=$peeraddr"
		fi
	fi

	echo "rule=$rule" > /tmp/map-$cfg.rules
	RULE_DATA=$(mapcalc ${tunlink:-\*} $rule)
	if [ "$?" != 0 ]; then
		proto_notify_error "$cfg" "INVALID_MAP_RULE"
		proto_block_restart "$cfg"
		return
	fi

	echo "$RULE_DATA" >> /tmp/map-$cfg.rules
	eval $RULE_DATA

	if [ -z "$RULE_BMR" ]; then
		proto_notify_error "$cfg" "NO_MATCHING_PD"
		proto_block_restart "$cfg"
		return
	fi

	k=$RULE_BMR
	if [ "$type" = "lw4o6" -o "$type" = "map-e" ]; then
		proto_init_update "$link" 1
		proto_add_ipv4_address $(eval "echo \$RULE_${k}_IPV4ADDR") "" "" ""

		proto_add_tunnel
		json_add_string mode ipip6
		json_add_int mtu "${mtu:-1280}"
		json_add_int ttl "${ttl:-64}"
		json_add_string local $(eval "echo \$RULE_${k}_IPV6ADDR")
		json_add_string remote $(eval "echo \$RULE_${k}_BR")
		json_add_string link $(eval "echo \$RULE_${k}_PD6IFACE")
		json_add_object "data"
			[ -n "$encaplimit" ] && json_add_string encaplimit "$encaplimit"
			if [ "$type" = "map-e" ]; then
				json_add_array "fmrs"
				for i in $(seq $RULE_COUNT); do
					[ "$(eval "echo \$RULE_${i}_FMR")" != 1 ] && continue
					json_add_object ""
					json_add_string prefix6 "$(eval "echo \$RULE_${i}_IPV6PREFIX")/$(eval "echo \$RULE_${i}_PREFIX6LEN")"
					json_add_string prefix4 "$(eval "echo \$RULE_${i}_IPV4PREFIX")/$(eval "echo \$RULE_${i}_PREFIX4LEN")"
					json_add_int ealen $(eval "echo \$RULE_${i}_EALEN")
					json_add_int offset $(eval "echo \$RULE_${i}_OFFSET")
					json_close_object
				done
				json_close_array
			fi
		json_close_object


		proto_close_tunnel
	elif [ "$type" = "map-t" -a -f "/proc/net/nat46/control" ]; then
		proto_init_update "$link" 1
		local style="MAP"
		[ "$LEGACY" = 1 ] && style="MAP0"

		echo add $link > /proc/net/nat46/control
		local cfgstr="local.style $style local.v4 $(eval "echo \$RULE_${k}_IPV4PREFIX")/$(eval "echo \$RULE_${k}_PREFIX4LEN")"
		cfgstr="$cfgstr local.v6 $(eval "echo \$RULE_${k}_IPV6PREFIX")/$(eval "echo \$RULE_${k}_PREFIX6LEN")"
		cfgstr="$cfgstr local.ea-len $(eval "echo \$RULE_${k}_EALEN") local.psid-offset $(eval "echo \$RULE_${k}_OFFSET")"
		cfgstr="$cfgstr remote.v4 0.0.0.0/0 remote.v6 $(eval "echo \$RULE_${k}_DMR") remote.style RFC6052 remote.ea-len 0 remote.psid-offset 0"
		echo config $link $cfgstr > /proc/net/nat46/control

		for i in $(seq $RULE_COUNT); do
			[ "$(eval "echo \$RULE_${i}_FMR")" != 1 ] && continue
			local cfgstr="remote.style $style remote.v4 $(eval "echo \$RULE_${i}_IPV4PREFIX")/$(eval "echo \$RULE_${i}_PREFIX4LEN")"
			cfgstr="$cfgstr remote.v6 $(eval "echo \$RULE_${i}_IPV6PREFIX")/$(eval "echo \$RULE_${i}_PREFIX6LEN")"
			cfgstr="$cfgstr remote.ea-len $(eval "echo \$RULE_${i}_EALEN") remote.psid-offset $(eval "echo \$RULE_${i}_OFFSET")"
			echo insert $link $cfgstr > /proc/net/nat46/control
		done
	else
		proto_notify_error "$cfg" "UNSUPPORTED_TYPE"
		proto_block_restart "$cfg"
	fi

	proto_add_ipv4_route "0.0.0.0" 0
	proto_add_data
	[ -n "$zone" ] && json_add_string zone "$zone"

	json_add_array firewall
	  if [ -z "$(eval "echo \$RULE_${k}_PORTSETS")" ]; then
	    json_add_object ""
	      json_add_string type nat
	      json_add_string target SNAT
	      json_add_string family inet
	      json_add_string snat_ip $(eval "echo \$RULE_${k}_IPV4ADDR")
	    json_close_object
	  else
	    for portset in $(eval "echo \$RULE_${k}_PORTSETS"); do
              for proto in icmp tcp udp; do
	        json_add_object ""
	          json_add_string type nat
	          json_add_string target SNAT
	          json_add_string family inet
	          json_add_string proto "$proto"
                  json_add_boolean connlimit_ports 1
                  json_add_string snat_ip $(eval "echo \$RULE_${k}_IPV4ADDR")
                  json_add_string snat_port "$portset"
	        json_close_object
              done
	    done
	  fi
	  if [ "$type" = "map-t" ]; then
		[ -z "$zone" ] && zone=$(fw3 -q network $iface 2>/dev/null)

		[ -n "$zone" ] && {
			json_add_object ""
				json_add_string type rule
				json_add_string family inet6
				json_add_string proto all
				json_add_string direction in
				json_add_string dest "$zone"
				json_add_string src "$zone"
				json_add_string src_ip $(eval "echo \$RULE_${k}_IPV6ADDR")
				json_add_string target ACCEPT
			json_close_object
			json_add_object ""
				json_add_string type rule
				json_add_string family inet6
				json_add_string proto all
				json_add_string direction out
				json_add_string dest "$zone"
				json_add_string src "$zone"
				json_add_string dest_ip $(eval "echo \$RULE_${k}_IPV6ADDR")
				json_add_string target ACCEPT
			json_close_object
		}
		proto_add_ipv6_route $(eval "echo \$RULE_${k}_IPV6ADDR") 128
	  fi
	json_close_array
	proto_close_data

	proto_send_update "$cfg"

	if [ "$type" = "lw4o6" -o "$type" = "map-e" ]; then
		json_init
		json_add_string name "${cfg}_"
		json_add_string ifname "@$(eval "echo \$RULE_${k}_PD6IFACE")"
		json_add_string proto "static"
		json_add_array ip6addr
		json_add_string "" "$(eval "echo \$RULE_${k}_IPV6ADDR")"
		json_close_array
		json_close_object
		ubus call network add_dynamic "$(json_dump)"
	fi
}

proto_map_teardown() {
	local cfg="$1"
	local link="map-$cfg"

	json_get_var type type

	[ -z "$type" ] && type="map-e"

	case "$type" in
		"map-e"|"lw4o6") ifdown "${cfg}_" ;;
		"map-t") [ -f "/proc/net/nat46/control" ] && echo del $link > /proc/net/nat46/control ;;
	esac

	rm -f /tmp/map-$cfg.rules
}

proto_map_init_config() {
	no_device=1
	available=1

	proto_config_add_string "rule"
	proto_config_add_string "ipaddr"
	proto_config_add_int "ip4prefixlen"
	proto_config_add_string "ip6prefix"
	proto_config_add_int "ip6prefixlen"
	proto_config_add_string "peeraddr"
	proto_config_add_int "ealen"
	proto_config_add_int "psidlen"
	proto_config_add_int "psid"
	proto_config_add_int "offset"

	proto_config_add_string "tunlink"
	proto_config_add_int "mtu"
	proto_config_add_int "ttl"
	proto_config_add_string "zone"
	proto_config_add_string "encaplimit"
}

[ -n "$INCLUDE_ONLY" ] || {
        add_protocol map
}
