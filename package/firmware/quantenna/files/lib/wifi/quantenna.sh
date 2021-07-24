#!/bin/sh
. /lib/netifd/quantenna.sh

append DRIVERS "quantenna"

detect_quantenna() {
	devidx=0
	config_load wireless
	while :; do
		config_get type "radio$devidx" type
		[ -n "$type" ] || break
		# the qcsapi does not support more than one module in a system
		[ "$type" = "quantenna" ] && return
		devidx=$(($devidx + 1))
	done

	# find "all" - i.e. then single supported - Quantenna devices.
	# there is no reliable way to do this, as it can be virtually
	# any ethernet device
	phy=$(quantenna_device)
	[ -z "$phy" ] && return

	uci -q batch <<-EOF
		set wireless.radio${devidx}=wifi-device
		set wireless.radio${devidx}.type=quantenna
		set wireless.radio${devidx}.channel=${channel:-36}
		set wireless.radio${devidx}.hwmode=11ac
		set wireless.radio${devidx}.band=5g
		set wireless.radio${devidx}.htmode=VHT80
		set wireless.radio${devidx}.phy=${phy}
		set wireless.radio${devidx}.disabled=0

		set wireless.default_radio${devidx}=wifi-iface
		set wireless.default_radio${devidx}.device=radio${devidx}
		set wireless.default_radio${devidx}.network=lan
		set wireless.default_radio${devidx}.mode=ap
		set wireless.default_radio${devidx}.ssid=OpenWrt-5G
		set wireless.default_radio${devidx}.encryption=psk2+ccmp
		set wireless.default_radio${devidx}.key=12345678
EOF
	uci -q commit wireless
}		
