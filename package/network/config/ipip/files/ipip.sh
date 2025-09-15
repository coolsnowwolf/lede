#!/bin/sh

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. /lib/functions/network.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

proto_ipip_setup() {
	local cfg="$1"
	local remoteip

	local df ipaddr peeraddr tunlink ttl tos zone mtu
	json_get_vars df ipaddr peeraddr tunlink ttl tos zone mtu

	[ -z "$peeraddr" ] && {
		proto_notify_error "$cfg" "MISSING_PEER_ADDRESS"
		proto_block_restart "$cfg"
		return
	}

	remoteip=$(resolveip -t 10 -4 "$peeraddr")

	if [ -z "$remoteip" ]; then
		proto_notify_error "$cfg" "PEER_RESOLVE_FAIL"
		return
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
			return
		fi

		if ! network_get_ipaddr ipaddr "$wanif"; then
			proto_notify_error "$cfg" "NO_WAN_LINK"
			return
		fi
	}

	proto_init_update "ipip-$cfg" 1

	proto_add_tunnel
	json_add_string mode "ipip"
	json_add_int mtu "${mtu:-1280}"
	json_add_int ttl "${ttl:-64}"
	[ -n "$tos" ] && json_add_string tos "$tos"
	json_add_string local "$ipaddr"
	json_add_string remote "$peeraddr"
	[ -n "$tunlink" ] && json_add_string link "$tunlink"
	json_add_boolean df "${df:-1}"

	proto_close_tunnel

	proto_add_data
	[ -n "$zone" ] && json_add_string zone "$zone"
	proto_close_data

	proto_send_update "$cfg"
}

proto_ipip_teardown() {
	local cfg="$1"
}

proto_ipip_init_config() {
	no_device=1
	available=1

	proto_config_add_int "mtu"
	proto_config_add_int "ttl"
	proto_config_add_string "tos"
	proto_config_add_string "tunlink"
	proto_config_add_string "zone"
	proto_config_add_string "ipaddr"
	proto_config_add_string "peeraddr"
	proto_config_add_boolean "df"
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol ipip
}
