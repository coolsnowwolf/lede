#!/bin/sh

[ -x /usr/sbin/unetd ] || exit 0

. /lib/functions.sh
. /lib/functions/network.sh
. ../netifd-proto.sh

init_proto "$@"

proto_unet_init_config() {
	proto_config_add_string device
	proto_config_add_string type
	proto_config_add_string auth_key
	proto_config_add_string key
	proto_config_add_string file
	proto_config_add_int keepalive
	proto_config_add_string domain
	proto_config_add_boolean dht
	proto_config_add_array "tunnels:list(string)"
	proto_config_add_array "connect:list(string)"
	proto_config_add_array "peer_data:list(string)"
	no_device=1
	available=1
	no_proto_task=1
}

proto_unet_setup() {
	local config="$1"

	local device type key file keepalive domain tunnels
	json_get_vars device type auth_key key file keepalive domain dht
	json_get_values tunnels tunnels
	json_get_values connect connect
	json_get_values peer_data peer_data
	device="${device:-$config}"

	[ -n "$auth_key" ] && type="${type:-dynamic}"
	[ -n "$file" ] && type="${type:-file}"

	json_init
	json_add_string name "$device"
	json_add_string type "$type"
	json_add_string interface "$config"
	json_add_string auth_key "$auth_key"
	json_add_string key "$key"
	json_add_string file "$file"
	[ -n "$keepalive" ] && json_add_int keepalive "$keepalive"
	[ -n "$dht" ] && json_add_boolean dht "$dht"
	json_add_string domain "$domain"

	json_add_object tunnels
	for t in $tunnels; do
		local ifname="${t%%=*}"
		local service="${t#*=}"
		[ -n "$ifname" -a -n "$service" -a "$ifname" != "$t" ] || continue
		json_add_string "$ifname" "$service"
	done
	json_close_object

	json_add_array auth_connect
	for c in $connect; do
		json_add_string "" "$c"
	done
	json_close_array

	json_add_array peer_data
	for c in $peer_data; do
		json_add_string "" "$c"
	done
	json_close_array

	ip link del dev "$device" >/dev/null 2>&1
	ip link add dev "$device" type wireguard || {
		echo "Could not create wireguard device $device"
		proto_setup_failed "$config"
		exit 1
	}

	ubus call unetd network_add "$(json_dump)"
}

proto_unet_teardown() {
	local config="$1"
	local iface="$2"

	local device
	json_get_vars device
	device="${device:-$iface}"

	json_init
	json_add_string name "$device"

	ip link del dev "$device"

	ubus call unetd network_del "$(json_dump)"
}

add_protocol unet
