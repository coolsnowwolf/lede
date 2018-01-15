#!/bin/sh

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. /lib/functions/network.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

gre_generic_setup() {
	local cfg="$1"
	local mode="$2"
	local local="$3"
	local remote="$4"
	local link="$5"
	local mtu ttl tos zone ikey okey icsum ocsum iseqno oseqno multicast
	json_get_vars mtu ttl tos zone ikey okey icsum ocsum iseqno oseqno multicast

	[ -z "$zone" ] && zone="wan"
	[ -z "$multicast" ] && multicast=1

	proto_init_update "$link" 1

	proto_add_tunnel
	json_add_string mode "$mode"
	json_add_int mtu "${mtu:-1280}"
	[ -n "$df" ] && json_add_boolean df "$df"
	[ -n "$ttl" ] && json_add_int ttl "$ttl"
	[ -n "$tos" ] && json_add_string tos "$tos"
	json_add_boolean multicast "$multicast"
	json_add_string local "$local"
	json_add_string remote "$remote"
	[ -n "$tunlink" ] && json_add_string link "$tunlink"

	json_add_object 'data'
	[ -n "$ikey" ] && json_add_int ikey "$ikey"
	[ -n "$okey" ] && json_add_int okey "$okey"
	[ -n "$icsum" ] && json_add_boolean icsum "$icsum"
	[ -n "$ocsum" ] && json_add_boolean ocsum "$ocsum"
	[ -n "$iseqno" ] && json_add_boolean iseqno "$iseqno"
	[ -n "$oseqno" ] && json_add_boolean oseqno "$oseqno"
	json_close_object

	proto_close_tunnel

	proto_add_data
	[ -n "$zone" ] && json_add_string zone "$zone"
	proto_close_data

	proto_send_update "$cfg"
}

gre_setup() {
	local cfg="$1"
	local mode="$2"
	local remoteip

	local ipaddr peeraddr
	json_get_vars df ipaddr peeraddr tunlink

	[ -z "$peeraddr" ] && {
		proto_notify_error "$cfg" "MISSING_PEER_ADDRESS"
		proto_block_restart "$cfg"
		exit
	}

	remoteip=$(resolveip -t 10 -4 "$peeraddr")

	if [ -z "$remoteip" ]; then
		proto_notify_error "$cfg" "PEER_RESOLVE_FAIL"
		exit
	fi

	for ip in $remoteip; do
		peeraddr=$ip
		break
	done

	( proto_add_host_dependency "$cfg" "$peeraddr" "$tunlink" )

	[ -z "$ipaddr" ] && {
		local wanif="$tunlink"
		if [ -z $wanif ] && ! network_find_wan wanif; then
			proto_notify_error "$cfg" "NO_WAN_LINK"
			exit
		fi

		if ! network_get_ipaddr ipaddr "$wanif"; then
			proto_notify_error "$cfg" "NO_WAN_LINK"
			exit
		fi
	}

	[ -z "$df" ] && df="1"

	case "$mode" in
		gretapip)
			gre_generic_setup $cfg $mode $ipaddr $peeraddr "gre4t-$cfg"
			;;
		*)
			gre_generic_setup $cfg $mode $ipaddr $peeraddr "gre4-$cfg"
			;;
	esac
}

proto_gre_setup() {
	local cfg="$1"

	gre_setup $cfg "greip"
}

proto_gretap_setup() {
	local cfg="$1"

	local network
	json_get_vars network

	gre_setup $cfg "gretapip"

	json_init
	json_add_string name "gre4t-$cfg"
	json_add_boolean link-ext 0
	json_close_object

	for i in $network; do
		ubus call network.interface."$i" add_device "$(json_dump)"
	done
}

grev6_setup() {
	local cfg="$1"
	local mode="$2"
	local remoteip6

	local ip6addr peer6addr weakif
	json_get_vars ip6addr peer6addr tunlink weakif

	[ -z "$peer6addr" ] && {
		proto_notify_error "$cfg" "MISSING_PEER_ADDRESS"
		proto_block_restart "$cfg"
		exit
	}

	remoteip6=$(resolveip -t 10 -6 "$peer6addr")

	if [ -z "$remoteip6" ]; then
		proto_notify_error "$cfg" "PEER_RESOLVE_FAIL"
		exit
	fi

	for ip6 in $remoteip6; do
		peer6addr=$ip6
		break
	done

	( proto_add_host_dependency "$cfg" "$peer6addr" "$tunlink" )

	[ -z "$ip6addr" ] && {
		local wanif="$tunlink"
		if [ -z $wanif ] && ! network_find_wan6 wanif; then
			proto_notify_error "$cfg" "NO_WAN_LINK"
			exit
		fi

		if ! network_get_ipaddr6 ip6addr "$wanif"; then
			[ -z "$weakif" ] && weakif="lan"
			if ! network_get_ipaddr6 ip6addr "$weakif"; then
				proto_notify_error "$cfg" "NO_WAN_LINK"
				exit
			fi
		fi
	}

	case "$mode" in
		gretapip6)
			gre_generic_setup $cfg $mode $ip6addr $peer6addr "gre6t-$cfg"
			;;
		*)
			gre_generic_setup $cfg $mode $ip6addr $peer6addr "gre6-$cfg"
			;;
	esac
}

proto_grev6_setup() {
	local cfg="$1"

	grev6_setup $cfg "greip6"
}

proto_grev6tap_setup() {
	local cfg="$1"

	local network
	json_get_vars network

	grev6_setup $cfg "gretapip6"

	json_init
	json_add_string name "gre6t-$cfg"
	json_add_boolean link-ext 0
	json_close_object

	for i in $network; do
		ubus call network.interface."$i" add_device "$(json_dump)"
	done
}

gretap_generic_teardown() {
	local network
	json_get_vars network

	json_init
	json_add_string name "$1"
	json_add_boolean link-ext 0
	json_close_object

	for i in $network; do
		ubus call network.interface."$i" remove_device "$(json_dump)"
	done
}

proto_gre_teardown() {
	local cfg="$1"
}

proto_gretap_teardown() {
	local cfg="$1"

	gretap_generic_teardown "gre4t-$cfg"
}

proto_grev6_teardown() {
	local cfg="$1"
}

proto_grev6tap_teardown() {
	local cfg="$1"

	gretap_generic_teardown "gre6t-$cfg"
}

gre_generic_init_config() {
	no_device=1
	available=1

	proto_config_add_int "mtu"
	proto_config_add_int "ttl"
	proto_config_add_string "tos"
	proto_config_add_string "tunlink"
	proto_config_add_string "zone"
	proto_config_add_int "ikey"
	proto_config_add_int "okey"
	proto_config_add_boolean "icsum"
	proto_config_add_boolean "ocsum"
	proto_config_add_boolean "iseqno"
	proto_config_add_boolean "oseqno"
	proto_config_add_boolean "multicast"
}

proto_gre_init_config() {
	gre_generic_init_config
	proto_config_add_string "ipaddr"
	proto_config_add_string "peeraddr"
	proto_config_add_boolean "df"
}

proto_gretap_init_config() {
	proto_gre_init_config
	proto_config_add_string "network"
}

proto_grev6_init_config() {
	gre_generic_init_config
	proto_config_add_string "ip6addr"
	proto_config_add_string "peer6addr"
	proto_config_add_string "weakif"
}

proto_grev6tap_init_config() {
	proto_grev6_init_config
	proto_config_add_string "network"
}

[ -n "$INCLUDE_ONLY" ] || {
	[ -f /lib/modules/$(uname -r)/gre.ko ] && add_protocol gre
	[ -f /lib/modules/$(uname -r)/gre.ko ] && add_protocol gretap
	[ -f /lib/modules/$(uname -r)/ip6_gre.ko ] && add_protocol grev6
	[ -f /lib/modules/$(uname -r)/ip6_gre.ko ] && add_protocol grev6tap
}
