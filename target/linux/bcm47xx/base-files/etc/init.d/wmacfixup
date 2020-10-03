#!/bin/sh /etc/rc.common
# Copyright (C) 2010 OpenWrt.org

START=41

boot() {
	[ -d /sys/class/ieee80211 ] || exit

	commit=0

	fixup_wmac() {
		local cfg="$1"
		local cfmac

		config_get cfmac "$cfg" macaddr

		[ "$cfmac" != "00:90:4c:5f:00:2a" ] || {
			local nvmac="$(nvram get il0macaddr 2>/dev/null)"
			[ -n "$nvmac" ] && [ "$nvmac != "$cfmac ] && {
				uci set wireless.$cfg.macaddr="$nvmac"
				commit=1
			}
		}
	}

	config_load wireless
	config_foreach fixup_wmac wifi-device

	[ "$commit" = 1 ] && uci commit wireless
}

start() { :; }
stop()  { :; }
