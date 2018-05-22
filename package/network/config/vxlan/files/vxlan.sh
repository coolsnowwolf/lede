#!/bin/sh

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. /lib/functions/network.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

vxlan_generic_setup() {
	local cfg="$1"
	local mode="$2"
	local local="$3"
	local remote="$4"

	local link="$cfg"

	local port vid ttl tos mtu macaddr zone rxcsum txcsum
	json_get_vars port vid ttl tos mtu macaddr zone rxcsum txcsum


	proto_init_update "$link" 1

	proto_add_tunnel
	json_add_string mode "$mode"

	[ -n "$tunlink" ] && json_add_string link "$tunlink"
	[ -n "$local" ] && json_add_string local "$local"
	[ -n "$remote" ] && json_add_string remote "$remote"

	[ -n "$ttl" ] && json_add_int ttl "$ttl"
	[ -n "$tos" ] && json_add_string tos "$tos"
	[ -n "$mtu" ] && json_add_int mtu "$mtu"

	json_add_object 'data'
	[ -n "$port" ] && json_add_int port "$port"
	[ -n "$vid" ] && json_add_int id "$vid"
	[ -n "$macaddr" ] && json_add_string macaddr "$macaddr"
	[ -n "$rxcsum" ] && json_add_boolean rxcsum "$rxcsum"
	[ -n "$txcsum" ] && json_add_boolean txcsum "$txcsum"
	json_close_object

	proto_close_tunnel

	proto_add_data
	[ -n "$zone" ] && json_add_string zone "$zone"
	proto_close_data

	proto_send_update "$cfg"
}

proto_vxlan_setup() {
	local cfg="$1"

	local ipaddr peeraddr
	json_get_vars ipaddr peeraddr tunlink

	[ -z "$peeraddr" ] && {
		proto_notify_error "$cfg" "MISSING_ADDRESS"
		proto_block_restart "$cfg"
		exit
	}

	( proto_add_host_dependency "$cfg" '' "$tunlink" )

	[ -z "$ipaddr" ] && {
		local wanif="$tunlink"
		if [ -z "$wanif" ] && ! network_find_wan wanif; then
			proto_notify_error "$cfg" "NO_WAN_LINK"
			exit
		fi

		if ! network_get_ipaddr ipaddr "$wanif"; then
			proto_notify_error "$cfg" "NO_WAN_LINK"
			exit
		fi
	}

	vxlan_generic_setup "$cfg" 'vxlan' "$ipaddr" "$peeraddr"
}

proto_vxlan6_setup() {
	local cfg="$1"

	local ip6addr peer6addr
	json_get_vars ip6addr peer6addr tunlink

	[ -z "$peer6addr" ] && {
		proto_notify_error "$cfg" "MISSING_ADDRESS"
		proto_block_restart "$cfg"
		exit
	}

	( proto_add_host_dependency "$cfg" '' "$tunlink" )

	[ -z "$ip6addr" ] && {
		local wanif="$tunlink"
		if [ -z "$wanif" ] && ! network_find_wan6 wanif; then
			proto_notify_error "$cfg" "NO_WAN_LINK"
			exit
		fi

		if ! network_get_ipaddr6 ip6addr "$wanif"; then
			proto_notify_error "$cfg" "NO_WAN_LINK"
			exit
		fi
	}

	vxlan_generic_setup "$cfg" 'vxlan6' "$ip6addr" "$peer6addr"
}

proto_vxlan_teardown() {
	local cfg="$1"
}

proto_vxlan6_teardown() {
	local cfg="$1"
}

vxlan_generic_init_config() {
	no_device=1
	available=1

	proto_config_add_string "tunlink"
	proto_config_add_string "zone"

	proto_config_add_int "vid"
	proto_config_add_int "port"
	proto_config_add_int "ttl"
	proto_config_add_int "tos"
	proto_config_add_int "mtu"
	proto_config_add_string "macaddr"
}

proto_vxlan_init_config() {
	vxlan_generic_init_config
	proto_config_add_string "ipaddr"
	proto_config_add_string "peeraddr"
}

proto_vxlan6_init_config() {
	vxlan_generic_init_config
	proto_config_add_string "ip6addr"
	proto_config_add_string "peer6addr"
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol vxlan
	add_protocol vxlan6
}
