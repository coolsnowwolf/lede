#!/bin/sh

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

proto_directip_init_config() {
	available=1
	no_device=1
	proto_config_add_string "device:device"
	proto_config_add_string "apn"
	proto_config_add_string "pincode"
	proto_config_add_string "auth"
	proto_config_add_string "username"
	proto_config_add_string "password"
	proto_config_add_defaults
}

proto_directip_setup() {
	local interface="$1"
	local chat devpath devname

	local device apn pincode ifname auth username password $PROTO_DEFAULT_OPTIONS
	json_get_vars device apn pincode auth username password $PROTO_DEFAULT_OPTIONS

	[ -n "$ctl_device" ] && device=$ctl_device

	device="$(readlink -f $device)"
	[ -e "$device" ] || {
		proto_notify_error "$interface" NO_DEVICE
		proto_set_available "$interface" 0
		return 1
	}

	devname="$(basename "$device")"
	devpath="$(readlink -f /sys/class/tty/$devname/device)"
	ifname="$( ls "$devpath"/../../*/net )"

	[ -n "$ifname" ] || {
		proto_notify_error "$interface" NO_IFNAME
		proto_set_available "$interface" 0
		return 1
	}

	gcom -d "$device" -s /etc/gcom/getcardinfo.gcom | grep -q "Sierra Wireless" || {
		proto_notify_error "$interface" BAD_DEVICE
		proto_block_restart "$interface"
		return 1
	}

	if [ -n "$pincode" ]; then
		PINCODE="$pincode" gcom -d "$device" -s /etc/gcom/setpin.gcom || {
			proto_notify_error "$interface" PIN_FAILED
			proto_block_restart "$interface"
			return 1
		}
	fi
	# wait for carrier to avoid firmware stability bugs
	gcom -d "$device" -s /etc/gcom/getcarrier.gcom || return 1

	local auth_type=0
	case $auth in
	pap) auth_type=1;;
	chap) auth_type=2;;
	esac

	USE_APN="$apn" USE_USER="$username" USE_PASS="$password" USE_AUTH="$auth_type" \
			gcom -d "$device" -s /etc/gcom/directip.gcom || {
		proto_notify_error "$interface" CONNECT_FAILED
		proto_block_restart "$interface"
		return 1
	}

	logger -p daemon.info -t "directip[$$]" "Connected, starting DHCP"
	proto_init_update "$ifname" 1
	proto_send_update "$interface"

	json_init
	json_add_string name "${interface}_4"
	json_add_string ifname "@$interface"
	json_add_string proto "dhcp"
	proto_add_dynamic_defaults
	ubus call network add_dynamic "$(json_dump)"

	json_init
	json_add_string name "${interface}_6"
	json_add_string ifname "@$interface"
	json_add_string proto "dhcpv6"
	json_add_string extendprefix 1
	proto_add_dynamic_defaults
	ubus call network add_dynamic "$(json_dump)"

	return 0
}

proto_directip_teardown() {
	local interface="$1"

	local device
	json_get_vars device

	[ -n "$ctl_device" ] && device=$ctl_device

	gcom -d "$device" -s /etc/gcom/directip-stop.gcom || proto_notify_error "$interface" CONNECT_FAILED

	proto_init_update "*" 0
	proto_send_update "$interface"
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol directip
}
