#!/bin/sh  /etc/rc.common
#
# Copyright (c) 2014, 2019-2020 The Linux Foundation. All rights reserved.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

# The shebang above has an extra space intentially to avoid having
# openwrt build scripts automatically enable this package starting
# at boot.

START=19

get_front_end_mode() {
	config_load "ecm"
	config_get front_end global acceleration_engine "auto"

	case $front_end in
	auto)
		echo '0'
		;;
	nss)
		echo '1'
		;;
	*)
		echo 'uci_option_acceleration_engine is invalid'
	esac
}

support_bridge() {
	# NSS support bridge acceleration
	[ -d /sys/kernel/debug/ecm/ecm_nss_ipv4 ] && [ -d /sys/kernel/debug/ecm/ecm_nss_ipv6 ] && return 0
}

enable_bridge_filtering() {
	sysctl -w net.bridge.bridge-nf-call-arptables=0
	sysctl -w net.bridge.bridge-nf-call-iptables=0
	sysctl -w net.bridge.bridge-nf-call-ip6tables=0

	if ([ -z "$(grep "net.bridge.bridge-nf-call-arptables=0" /etc/sysctl.d/qca-nss-ecm.conf)" ] && \
		[ -z "$(grep "net.bridge.bridge-nf-call-iptables=0" /etc/sysctl.d/qca-nss-ecm.conf)" ] && \
		[ -z "$(grep "net.bridge.bridge-nf-call-ip6tables=0" /etc/sysctl.d/qca-nss-ecm.conf)" ] \
	); then
		echo 'net.bridge.bridge-nf-call-arptables=0' >> /etc/sysctl.d/qca-nss-ecm.conf
		echo 'net.bridge.bridge-nf-call-iptables=0' >> /etc/sysctl.d/qca-nss-ecm.conf
		echo 'net.bridge.bridge-nf-call-ip6tables=0' >> /etc/sysctl.d/qca-nss-ecm.conf
	fi
}

disable_bridge_filtering() {
	sysctl -w net.bridge.bridge-nf-call-arptables=0
	sysctl -w net.bridge.bridge-nf-call-iptables=0
	sysctl -w net.bridge.bridge-nf-call-ip6tables=0

	sed '/net.bridge.bridge-nf-call-arptables/d' -i /etc/sysctl.d/qca-nss-ecm.conf
	sed '/net.bridge.bridge-nf-call-iptables/d' -i /etc/sysctl.d/qca-nss-ecm.conf
	sed '/net.bridge.bridge-nf-call-ip6tables/d' -i /etc/sysctl.d/qca-nss-ecm.conf
}

load_ecm() {
	[ -d /sys/module/ecm ] || {
		insmod ecm front_end_selection=$(get_front_end_mode)
		echo 1 > /sys/kernel/debug/ecm/ecm_classifier_default/accel_delay_pkts
	}

	support_bridge && enable_bridge_filtering
}

unload_ecm() {
	disable_bridge_filtering

	if [ -d /sys/module/ecm ]; then
		#
		# Stop ECM frontends
		#
		echo 1 > /sys/kernel/debug/ecm/front_end_ipv4_stop
		echo 1 > /sys/kernel/debug/ecm/front_end_ipv6_stop

		#
		# Defunct the connections
		#
	    echo 1 > /sys/kernel/debug/ecm/ecm_db/defunct_all
		sleep 5

		rmmod ecm
		sleep 1
	fi
}

start() {
	load_ecm

	# If the acceleration engine is NSS, enable wifi redirect
	[ -d /sys/kernel/debug/ecm/ecm_nss_ipv4 ] && sysctl -w dev.nss.general.redirect=1

	# If bridge filtering is enabled, apply and persist the sysctl flags
	local bridge_filtering_enabled="$(uci_get ecm @general[0] enable_bridge_filtering)"
	if [ "$bridge_filtering_enabled" -eq 1 ]; then
		echo "Bridge filtering is enabled in the ECM config, this will cause issues with NAT loopback!"
		enable_bridge_filtering
	fi

	if [ -d /sys/module/qca_ovsmgr ]; then
		insmod ecm_ovs
	fi
}

stop() {
	# If ECM is already not loaded, just return
	if [ ! -d /sys/module/ecm ]; then
		return
	fi

	# If the acceleration engine is NSS, disable wifi redirect
	[ -d /sys/kernel/debug/ecm/ecm_nss_ipv4 ] && sysctl -w dev.nss.general.redirect=0

	# If bridge filtering is enabled, reset the sysctl flags
	local bridge_filtering_enabled="$(uci_get ecm @general[0] enable_bridge_filtering)"
	if [ "$bridge_filtering_enabled" -eq 1 ]; then
		disable_bridge_filtering
	fi

	if [ -d /sys/module/ecm_ovs ]; then
		rmmod ecm_ovs
	fi

	unload_ecm
}
