#!/bin/sh /etc/rc.common
# Copyright (C) 2018 OpenWrt.org

START=99
USE_PROCD=1
PROG=/usr/sbin/omcproxy

# Uncomment to enable verbosity
#OPTIONS="-v"
PROXIES=""

omcproxy_add_proxy() {
	local proxy scope uplink updevice downlinks

	config_get uplink $1 uplink
	[ -n "$uplink" ] || return

	network_get_device updevice "$uplink" || {
		procd_append_param error "$uplink is not up"
		return;
	}

	config_get downlinks $1 downlink
	for downlink in $downlinks; do
		local device

		network_get_device device "$downlink" || {
			procd_append_param error "$downlink is not up"
			continue;
		}

		proxy="$proxy,$device"

		# Disable in-kernel querier while ours is active, default is 1.
		[ -f /sys/class/net/$device/bridge/multicast_querier ] && \
			echo 0 > /sys/class/net/$device/bridge/multicast_querier
	done

	[ -n "$proxy" ] || return 0

	config_get scope $1 scope
	[ -n "$scope" ] && proxy="$proxy,scope=$scope"

	PROXIES="$PROXIES $updevice$proxy"
}

omcproxy_add_network_triggers() {
	local uplink downlinks

	config_get uplink $1 uplink
	config_get downlinks $1 downlink

	for link in $uplink $downlinks; do
		local duplicate=0

		for l in $LINKS; do
			[ "$l" = "$link" ] && duplicate=1
		done

		[ "$duplicate" = 0 ] && {
			LINKS="$LINKS $link"
			procd_add_interface_trigger "interface.*" $link /etc/init.d/omcproxy restart
		}
	done
}

omcproxy_add_firewall_rules() {
	local uplink downlinks

	config_get uplink $1 uplink
	config_get downlinks $1 downlink

	upzone=$(fw3 -q network $uplink 2>/dev/null)
	[ -n "$upzone" ] || return 0

	json_add_object ""
	json_add_string type rule
	json_add_string src "$upzone"
	json_add_string family ipv4
	json_add_string proto igmp
	json_add_string target ACCEPT
	json_close_object

	json_add_object ""
	json_add_string type rule
	json_add_string family ipv6
	json_add_string src "$upzone"
	json_add_string proto icmp
	json_add_string src_ip fe80::/10
	json_add_array icmp_type
		json_add_string "" 130/0
		json_add_string "" 131/0
		json_add_string "" 132/0
		json_add_string "" 143/0
	json_close_array
	json_add_string target ACCEPT
	json_close_object

	for downlink in $downlinks; do
		downzone=$(fw3 -q network $downlink 2>/dev/null)
		[ -n "$downzone" ] || continue

		json_add_object ""
		json_add_string type rule
		json_add_string src "$upzone"
		json_add_string dest "$downzone"
		json_add_string family ipv4
		json_add_string proto udp
		json_add_string dest_ip "224.0.0.0/4"
		json_add_string target ACCEPT
		json_close_object

		json_add_object ""
		json_add_string type rule
		json_add_string src "$upzone"
		json_add_string dest "$downzone"
		json_add_string family ipv6
		json_add_string proto udp
		json_add_string dest_ip "ff00::/8"
		json_add_string target ACCEPT
		json_close_object
	done
}

service_triggers() {
	LINKS=""

	procd_add_reload_trigger "omcproxy"
	config_foreach omcproxy_add_network_triggers proxy
}

start_service() {
	. /lib/functions/network.sh

	config_load omcproxy

	config_foreach omcproxy_add_proxy proxy
	[ -n "$PROXIES" ] || return 0

	procd_open_instance
	procd_set_param command $PROG
	[ -n "$OPTIONS" ] && procd_append_param command $OPTIONS
	procd_append_param command $PROXIES
	procd_set_param respawn

	procd_open_data

	json_add_array firewall
	config_foreach omcproxy_add_firewall_rules proxy
	json_close_array

	procd_close_data

	procd_close_instance

	# Increase maximum IPv4 group memberships per socket, default is 100.
	echo 128 > /proc/sys/net/ipv4/igmp_max_memberships
}

service_started() {
	procd_set_config_changed firewall
}

stop_service() {
	procd_set_config_changed firewall
}
