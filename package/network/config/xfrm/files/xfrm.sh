#!/bin/sh

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. /lib/functions/network.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

proto_xfrm_setup() {
	local cfg="$1"
	local mode="xfrm"

	local tunlink ifid mtu zone
	json_get_vars tunlink ifid mtu zone

	[ -z "$tunlink" ] && {
		proto_notify_error "$cfg" NO_TUNLINK
		proto_block_restart "$cfg"
		exit
	}

	[ -z "$ifid" ] && {
		proto_notify_error "$cfg" NO_IFID
		proto_block_restart "$cfg"
		exit
	}

	( proto_add_host_dependency "$cfg" '' "$tunlink" )

	proto_init_update "$cfg" 1

	proto_add_tunnel
	json_add_string mode "$mode"
	json_add_int mtu "${mtu:-1280}"

	json_add_string link "$tunlink"

	json_add_object 'data'
	[ -n "$ifid" ] && json_add_int ifid "$ifid"
	json_close_object

	proto_close_tunnel

	proto_add_data
	[ -n "$zone" ] && json_add_string zone "$zone"
	proto_close_data

	proto_send_update "$cfg"
}

proto_xfrm_teardown() {
	local cfg="$1"
}

proto_xfrm_init_config() {
	no_device=1
	available=1

	proto_config_add_int "mtu"
	proto_config_add_string "tunlink"
	proto_config_add_string "zone"
	proto_config_add_int "ifid"
}


[ -n "$INCLUDE_ONLY" ] || {
	[ -f /lib/modules/$(uname -r)/xfrm_interface.ko -o -d /sys/module/xfrm_interface ] && add_protocol xfrm
}
