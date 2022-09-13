#!/bin/sh /etc/rc.common

START=25
USE_PROCD=1

start_service() {
	reload_service
}

service_triggers() {
	procd_add_reload_trigger "network"
	procd_add_reload_trigger "firewall"
	procd_add_reload_interface_trigger "lan"
}

reload_service() {
	local packet_steering="$(uci -q get network.@globals[0].packet_steering)"
	local num_cpus="$(grep -c "^processor.*:" /proc/cpuinfo)"
	local flow_offloading="$(uci -q get firewall.@defaults[0].flow_offloading)"
	local flow_offloading_hw="$(uci -q get firewall.@defaults[0].flow_offloading_hw)"

	# Any steering on 1 CPU (BCM47081) worsens network performance
	[ "$num_cpus" != 2 ] && return

	[ "$packet_steering" != 1 ] && {
		echo 0 > /sys/class/net/br-lan/queues/rx-0/rps_cpus
		echo 0 > /sys/class/net/eth0/queues/rx-0/rps_cpus
		return
	}

	if [ ${flow_offloading_hw:-0} -gt 0 ]; then
		# HW offloading
		echo 0 > /sys/class/net/br-lan/queues/rx-0/rps_cpus
		echo 0 > /sys/class/net/eth0/queues/rx-0/rps_cpus
	elif [ ${flow_offloading:-0} -gt 0 ]; then
		# SW offloading
		# br-lan setup doesn't seem to matter for offloading case
		echo 2 > /sys/class/net/eth0/queues/rx-0/rps_cpus
	else
		# Default
		echo 2 > /sys/class/net/br-lan/queues/rx-0/rps_cpus
		echo 0 > /sys/class/net/eth0/queues/rx-0/rps_cpus
	fi
}
