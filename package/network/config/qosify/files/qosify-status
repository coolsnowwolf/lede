#!/bin/sh
. /usr/share/libubox/jshn.sh

dev_status() {
	tc -s qdisc sh dev "$1" root
	echo
}

common_status() {
	json_get_vars ifname ingress egress

	[ -n "$ifname" ] || return

	[ "$egress" -gt 0 ] && {
		echo "egress status:"
		dev_status "$ifname"
	}
	[ "$ingress" -gt 0 ] && {
		echo "ingress status:"
		dev_status "$(printf %.16s "ifb-$ifname")"
	}
}

is_active() {
	json_get_vars active

	[ "${active:-0}" -gt 0 ]
}

device_status() {
	local name="$2"

	json_select "$name"

	if is_active; then
		status="active"
	else
		status="not found"
	fi

	echo "===== device $name: $status ====="

	is_active && common_status

	json_select ..
}

interface_status() {
	local name="$2"

	json_select "$name"

	if is_active; then
		status="active"
	elif ubus -S -t 0 wait_for "network.interface.$name"; then
		status="down"
	else
		status="not found"
	fi

	echo "===== interface $name: $status ====="

	is_active && common_status

	json_select ..
}

json_load "$(ubus call qosify status)"
json_for_each_item device_status devices
json_for_each_item interface_status interfaces
