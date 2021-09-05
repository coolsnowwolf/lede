#!/bin/sh

. /lib/functions.sh
. ../netifd-proto.sh
init_proto "$@"

proto_autoip_setup() {
	local config="$1"
	local iface="$2"

	proto_export "INTERFACE=$config"
	proto_run_command "$config" avahi-autoipd "$iface"
}

proto_autoip_teardown() {
	local interface="$1"
	proto_kill_command "$interface"
}

add_protocol autoip
