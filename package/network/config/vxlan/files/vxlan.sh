#!/bin/sh

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. /lib/functions/network.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

proto_vxlan_setup_peer() {
	type bridge &> /dev/null || {
		proto_notify_error "$cfg" "MISSING_BRIDGE_COMMAND"
		exit
	}

	local peer_config="$1"

	local vxlan
	local lladdr
	local dst
	local src_vni
	local vni
	local port
	local via

	config_get vxlan   "${peer_config}" "vxlan"
	config_get lladdr  "${peer_config}" "lladdr"
	config_get dst     "${peer_config}" "dst"
	config_get src_vni "${peer_config}" "src_vni"
	config_get vni     "${peer_config}" "vni"
	config_get port    "${peer_config}" "port"
	config_get via     "${peer_config}" "via"

	[ "$cfg" = "$vxlan" ] || {
		# This peer section belongs to another device
		return
	}

	[ -n "${dst}" ] || {
		proto_notify_error "$cfg" "MISSING_PEER_ADDRESS"
		exit
	}

	bridge fdb append \
		${lladdr:-00:00:00:00:00:00} \
		dev ${cfg}                   \
		dst ${dst}                   \
		${src_vni:+src_vni $src_vni} \
		${vni:+vni $vni}             \
		${port:+port $port}          \
		${via:+via $via}
}

vxlan_generic_setup() {
	local cfg="$1"
	local mode="$2"
	local local="$3"
	local remote="$4"

	local link="$cfg"

	local port vid ttl tos mtu macaddr zone rxcsum txcsum srcportmin srcportmax ageing maxaddress learning rsc proxy l2miss l3miss gbp
	json_get_vars port vid ttl tos mtu macaddr zone rxcsum txcsum srcportmin srcportmax ageing maxaddress learning rsc proxy l2miss l3miss gbp

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
	[ -n "$srcportmin" ] && json_add_int srcportmin "$srcportmin"
	[ -n "$srcportmax" ] && json_add_int srcportmax "$srcportmax"
	[ -n "$ageing" ] && json_add_int ageing "$ageing"
	[ -n "$maxaddress" ] && json_add_int maxaddress "$maxaddress"
	[ -n "$macaddr" ] && json_add_string macaddr "$macaddr"
	[ -n "$rxcsum" ] && json_add_boolean rxcsum "$rxcsum"
	[ -n "$txcsum" ] && json_add_boolean txcsum "$txcsum"
	[ -n "$learning" ] && json_add_boolean learning "$learning"
	[ -n "$rsc" ] && json_add_boolean rsc "$rsc"
	[ -n "$proxy" ] && json_add_boolean proxy "$proxy"
	[ -n "$l2miss" ] && json_add_boolean l2miss "$l2miss"
	[ -n "$l3miss" ] && json_add_boolean l3miss "$l3miss"
	[ -n "$gbp" ] && json_add_boolean gbp "$gbp"

	json_close_object

	proto_close_tunnel

	proto_add_data
	[ -n "$zone" ] && json_add_string zone "$zone"
	proto_close_data

	proto_send_update "$cfg"

	config_load network
	config_foreach proto_vxlan_setup_peer "vxlan_peer"
}

proto_vxlan_setup() {
	local cfg="$1"

	local ipaddr peeraddr
	json_get_vars ipaddr peeraddr tunlink

	( proto_add_host_dependency "$cfg" '' "$tunlink" )

	case "$ipaddr" in
		"auto"|"")
			ipaddr="0.0.0.0"
			;;
	esac

	vxlan_generic_setup "$cfg" 'vxlan' "$ipaddr" "$peeraddr"
}

proto_vxlan6_setup() {
	local cfg="$1"

	local ip6addr peer6addr
	json_get_vars ip6addr peer6addr tunlink

	( proto_add_host_dependency "$cfg" '' "$tunlink" )

	case "$ip6addr" in
		"auto"|"")
			# ensure tunnel via ipv6
			ip6addr="::"
			;;
	esac

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
	proto_config_add_int "srcportmin"
	proto_config_add_int "srcportmax"
	proto_config_add_int "ageing"
	proto_config_add_int "maxaddress"
	proto_config_add_boolean "rxcsum"
	proto_config_add_boolean "txcsum"
	proto_config_add_boolean "learning"
	proto_config_add_boolean "rsc"
	proto_config_add_boolean "proxy"
	proto_config_add_boolean "l2miss"
	proto_config_add_boolean "l3miss"
	proto_config_add_boolean "gbp"
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
