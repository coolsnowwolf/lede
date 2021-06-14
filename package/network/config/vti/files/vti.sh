#!/bin/sh

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. /lib/functions/network.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

vti_generic_setup() {
	local cfg="$1"
	local mode="$2"
	local local="$3"
	local remote="$4"
	local link="$5"
	local mtu zone ikey
	json_get_vars mtu zone ikey okey

	proto_init_update "$link" 1

	proto_add_tunnel
	json_add_string mode "$mode"
	json_add_int mtu "${mtu:-1280}"
	json_add_string local "$local"
	json_add_string remote "$remote"
	[ -n "$tunlink" ] && json_add_string link "$tunlink"

	json_add_object 'data'
	[ -n "$ikey" ] && json_add_int ikey "$ikey"
	[ -n "$okey" ] && json_add_int okey "$okey"
	json_close_object

	proto_close_tunnel

	proto_add_data
	[ -n "$zone" ] && json_add_string zone "$zone"
	proto_close_data

	proto_send_update "$cfg"
}

vti_setup() {
	local cfg="$1"
	local mode="$2"

	local ipaddr peeraddr
	json_get_vars df ipaddr peeraddr tunlink

	[ -z "$peeraddr" ] && {
		proto_notify_error "$cfg" "MISSING_ADDRESS"
		proto_block_restart "$cfg"
		exit
	}

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

	vti_generic_setup $cfg $mode $ipaddr $peeraddr "vti-$cfg"
}

proto_vti_setup() {
	local cfg="$1"

	vti_setup $cfg "vtiip"
}

vti6_setup() {
	local cfg="$1"
	local mode="$2"

	local ip6addr peer6addr weakif
	json_get_vars ip6addr peer6addr tunlink weakif

	[ -z "$peer6addr" ] && {
		proto_notify_error "$cfg" "MISSING_ADDRESS"
		proto_block_restart "$cfg"
		exit
	}

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

	vti_generic_setup $cfg $mode $ip6addr $peer6addr "vti6-$cfg"
}

proto_vti6_setup() {
	local cfg="$1"

	vti6_setup $cfg "vtiip6"
}

proto_vti_teardown() {
	local cfg="$1"
}

proto_vti6_teardown() {
	local cfg="$1"
}

vti_generic_init_config() {
	no_device=1
	available=1

	proto_config_add_int "mtu"
	proto_config_add_string "tunlink"
	proto_config_add_string "zone"
	proto_config_add_int "ikey"
	proto_config_add_int "okey"
}

proto_vti_init_config() {
	vti_generic_init_config
	proto_config_add_string "ipaddr"
	proto_config_add_string "peeraddr"
}

proto_vti6_init_config() {
	vti_generic_init_config
	proto_config_add_string "ip6addr"
	proto_config_add_string "peer6addr"
	proto_config_add_string "weakif"
}

[ -n "$INCLUDE_ONLY" ] || {
	[ -d /sys/module/ip_vti ] && add_protocol vti
	[ -d /sys/module/ip6_vti ] && add_protocol vti6
}
