#!/bin/sh
# Copyright (C) 2006 OpenWrt.org

. /lib/functions.sh
. /usr/share/libubox/jshn.sh

usage() {
	cat <<EOF
Usage: $0 [config|up|down|reconf|reload|status|isup]
enables (default), disables or configures devices not yet configured.
EOF
	exit 1
}

ubus_wifi_cmd() {
	local cmd="$1"
	local dev="$2"

	json_init
	[ -n "$2" ] && json_add_string device "$2"
	ubus call network.wireless "$1" "$(json_dump)"
}

wifi_isup() {
	local dev="$1"

	json_load "$(ubus_wifi_cmd "status" "$dev")"
	json_get_keys devices

	for device in $devices; do
		json_select "$device"
			json_get_var up up
			[ $up -eq 0 ] && return 1
		json_select ..
	done

	return 0
}

find_net_config() {(
	local vif="$1"
	local cfg
	local ifname

	config_get cfg "$vif" network

	[ -z "$cfg" ] && {
		include /lib/network
		scan_interfaces

		config_get ifname "$vif" ifname

		cfg="$(find_config "$ifname")"
	}
	[ -z "$cfg" ] && return 0
	echo "$cfg"
)}


bridge_interface() {(
	local cfg="$1"
	[ -z "$cfg" ] && return 0

	include /lib/network
	scan_interfaces

	for cfg in $cfg; do
		config_get iftype "$cfg" type
		[ "$iftype" = bridge ] && config_get "$cfg" ifname
		prepare_interface_bridge "$cfg"
		return $?
	done
)}

prepare_key_wep() {
	local key="$1"
	local hex=1

	echo -n "$key" | grep -qE "[^a-fA-F0-9]" && hex=0
	[ "${#key}" -eq 10 -a $hex -eq 1 ] || \
	[ "${#key}" -eq 26 -a $hex -eq 1 ] || {
		[ "${key:0:2}" = "s:" ] && key="${key#s:}"
		key="$(echo -n "$key" | hexdump -ve '1/1 "%02x" ""')"
	}
	echo "$key"
}

wifi_fixup_hwmode() {
	local device="$1"
	local default="$2"
	local hwmode hwmode_11n

	config_get channel "$device" channel
	config_get hwmode "$device" hwmode
	case "$hwmode" in
		11bg) hwmode=bg;;
		11a) hwmode=a;;
		11ad) hwmode=ad;;
		11b) hwmode=b;;
		11g) hwmode=g;;
		11n*)
			hwmode_11n="${hwmode##11n}"
			case "$hwmode_11n" in
				a|g) ;;
				default) hwmode_11n="$default"
			esac
			config_set "$device" hwmode_11n "$hwmode_11n"
		;;
		*)
			hwmode=
			if [ "${channel:-0}" -gt 0 ]; then
				if [ "${channel:-0}" -gt 14 ]; then
					hwmode=a
				else
					hwmode=g
				fi
			else
				hwmode="$default"
			fi
		;;
	esac
	config_set "$device" hwmode "$hwmode"
}

_wifi_updown() {
	for device in ${2:-$DEVICES}; do (
		config_get disabled "$device" disabled
		[ "$disabled" = "1" ] && {
			echo "'$device' is disabled"
			set disable
		}
		config_get iftype "$device" type
		if eval "type ${1}_$iftype" 2>/dev/null >/dev/null; then
			eval "scan_$iftype '$device'"
			eval "${1}_$iftype '$device'" || echo "$device($iftype): ${1} failed"
		elif [ ! -f /lib/netifd/wireless/$iftype.sh ]; then
			echo "$device($iftype): Interface type not supported"
		fi
	); done
}

wifi_updown() {
	cmd=down
	[ enable = "$1" ] && {
		_wifi_updown disable "$2"
		ubus_wifi_cmd "$cmd" "$2"
		ubus call network reload
		scan_wifi
		cmd=up
	}
	[ reconf = "$1" ] && {
		ubus call network reload
		scan_wifi
		cmd=reconf
	}
	ubus_wifi_cmd "$cmd" "$2"
	_wifi_updown "$@"
}

wifi_reload_legacy() {
	_wifi_updown "disable" "$1"
	scan_wifi
	_wifi_updown "enable" "$1"
}

wifi_reload() {
	ubus call network reload
	wifi_reload_legacy
}

wifi_detect_notice() {
	>&2 echo "WARNING: Wifi detect is deprecated. Use wifi config instead"
	>&2 echo "For more information, see commit 5f8f8a366136a07df661e31decce2458357c167a"
	exit 1
}

wifi_config() {
	[ ! -f /etc/config/wireless ] && touch /etc/config/wireless

	for driver in $DRIVERS; do (
		if eval "type detect_$driver" 2>/dev/null >/dev/null; then
			eval "detect_$driver" || echo "$driver: Detect failed" >&2
		else
			echo "$driver: Hardware detection not supported" >&2
		fi
	); done
}

start_net() {(
	local iface="$1"
	local config="$2"
	local vifmac="$3"

	[ -f "/var/run/$iface.pid" ] && kill "$(cat /var/run/${iface}.pid)" 2>/dev/null
	[ -z "$config" ] || {
		include /lib/network
		scan_interfaces
		for config in $config; do
			setup_interface "$iface" "$config" "" "$vifmac"
		done
	}
)}

set_wifi_up() {
	local cfg="$1"
	local ifname="$2"
	uci_set_state wireless "$cfg" up 1
	uci_set_state wireless "$cfg" ifname "$ifname"
}

set_wifi_down() {
	local cfg="$1"
	local vifs vif vifstr

	[ -f "/var/run/wifi-${cfg}.pid" ] &&
		kill "$(cat "/var/run/wifi-${cfg}.pid")" 2>/dev/null
	uci_revert_state wireless "$cfg"
	config_get vifs "$cfg" vifs
	for vif in $vifs; do
		uci_revert_state wireless "$vif"
	done
}

scan_wifi() {
	local cfgfile="$1"
	DEVICES=
	config_cb() {
		local type="$1"
		local section="$2"

		# section start
		case "$type" in
			wifi-device)
				append DEVICES "$section"
				config_set "$section" vifs ""
				config_set "$section" ht_capab ""
			;;
		esac

		# section end
		config_get TYPE "$CONFIG_SECTION" TYPE
		case "$TYPE" in
			wifi-iface)
				config_get device "$CONFIG_SECTION" device
				config_get vifs "$device" vifs
				append vifs "$CONFIG_SECTION"
				config_set "$device" vifs "$vifs"
			;;
		esac
	}
	config_load "${cfgfile:-wireless}"
}

DEVICES=
DRIVERS=
include /lib/wifi
scan_wifi

case "$1" in
	down) wifi_updown "disable" "$2";;
	detect) wifi_detect_notice ;;
	config) wifi_config ;;
	status) ubus_wifi_cmd "status" "$2";;
	isup) wifi_isup "$2"; exit $?;;
	reload) wifi_reload "$2";;
	reload_legacy) wifi_reload_legacy "$2";;
	--help|help) usage;;
	reconf) wifi_updown "reconf" "$2";;
	''|up) wifi_updown "enable" "$2";;
	*) usage; exit 1;;
esac
