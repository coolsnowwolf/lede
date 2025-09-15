#!/bin/sh /etc/rc.common
START=80

check_relay() {
	local cfg="$1"

	config_get_bool enabled "$cfg" enabled 1
	[ "$enabled" -gt 0 ] || return

	config_get dev1 "$cfg" dev1
	config_get dev2 "$cfg" dev2

	[ -d "/sys/kernel/debug/trelay/${dev1}-${dev2}" ] && return
	[ -d "/sys/class/net/${dev1}" -a -d "/sys/class/net/${dev2}" ] || return

	ip link set dev "$dev1" up
	ip link set dev "$dev2" up
	echo "${dev1}-${dev2},${dev1},${dev2}" > /sys/kernel/debug/trelay/add
}

start() {
	config_load trelay
	config_foreach check_relay trelay
	touch /var/run/trelay.active
}

stop() {
	rm -f /var/run/trelay.active
	for relay in /sys/kernel/debug/trelay/*; do
		[ -d "$relay" ] && echo > "$relay/remove"
	done
}
