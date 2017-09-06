#!/bin/sh /etc/rc.common
# Copyright (C) 2013 OpenWrt.org

START=61

USE_PROCD=1

find_nameserver() {
	. /lib/functions/network.sh

	local tmp
	if network_find_wan tmp && network_get_dnsserver tmp "$tmp"; then
		echo "${tmp%% *}"
		return 0
	fi

	return 1
}

start_instance() {
	local cfg="$1"
	local ipset ipset6 port dns

	config_get ipset "$cfg" ipset
	config_get ipset6 "$cfg" ipset6
	[ -n "$ipset$ipset6" ] || {
		echo "No ipset specified for instance $cfg" >&2
		return 1
	}

	config_get dns "$cfg" dns "$DEFNS"
	[ -n "$dns" ] || {
		echo "No DNS server specified for instance $cfg" >&2
		return 1
	}

	config_get port "$cfg" port $((PORT++))

	procd_open_instance
	procd_set_param command /usr/sbin/ipset-dns "$ipset" "$ipset6" "$port" "$dns"
	procd_set_param env NO_DAEMONIZE=1
	procd_set_param respawn
	procd_close_instance
}

service_triggers()
{
	procd_add_reload_trigger "ipset-dns"
}

start_service() {
	PORT=53001
	DEFNS="$(find_nameserver)"

	config_load ipset-dns
	config_foreach start_instance ipset-dns
}
