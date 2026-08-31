NETIFD_MAIN_DIR="${NETIFD_MAIN_DIR:-/lib/netifd}"

. /usr/share/libubox/jshn.sh
. $NETIFD_MAIN_DIR/utils.sh

CMD_UP=0
CMD_SET_DATA=1
CMD_PROCESS_ADD=2
CMD_PROCESS_KILL_ALL=3
CMD_SET_RETRY=4

add_driver() {
	return
}

wireless_setup_vif_failed() {
	local error="$1"
	echo "Interface $_w_iface setup failed: $error"
}

wireless_setup_failed() {
	local error="$1"

	echo "Device setup failed: $error"
	wireless_set_retry 0
}

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

_wdev_prepare_channel() {
	json_get_vars channel band hwmode htmode

	auto_channel=0
	enable_ht=0
	hwmode="${hwmode##11}"

	case "$channel" in
		""|0|auto)
			channel=0
			auto_channel=1
		;;
		[0-9]*) ;;
		*)
			wireless_setup_failed "INVALID_CHANNEL"
		;;
	esac

	case "$hwmode" in
		a|b|g|ad) ;;
		*)
			if [ "$channel" -gt 14 ]; then
				hwmode=a
			else
				hwmode=g
			fi
		;;
	esac

	case "$band" in
		2g) hwmode=g;;
		5g|6g) hwmode=a;;
		60g) hwmode=ad;;
		*)
			case "$hwmode" in
				*a) band=5g;;
				*ad) band=60g;;
				*b|*g) band=2g;;
			esac
		;;
	esac

	case "$htmode" in
		HE*|EHT*) wpa3_cipher="GCMP-256 ";;
		*) wpa3_cipher="";;
	esac
}

_wdev_handler() {
	json_load "$data"

	json_select config
	_wdev_prepare_channel
	json_select ..

	eval "drv_$1_$2 \"$interface\""
}

_wdev_msg_call() {
	local old_cb

	json_set_namespace wdev old_cb
	"$@"
	json_set_namespace $old_cb
}

_wdev_wrapper() {
	while [ -n "$1" ]; do
		eval "$1() { _wdev_msg_call _$1 \"\$@\"; }"
		shift
	done
}

_wdev_notify_init() {
	local command="$1"; shift;

	json_init
	json_add_int "command" "$command"
	json_add_string "device" "$__netifd_device"
	while [ -n "$1" ]; do
		local name="$1"; shift
		local value="$1"; shift
		json_add_string "$name" "$value"
	done
	json_add_object "data"
}

_wdev_notify() {
	local options="$1"

	json_close_object
	ubus $options call network.wireless notify "$(json_dump)"
}

_wdev_add_variables() {
	while [ -n "$1" ]; do
		local var="${1%%=*}"
		local val="$1"
		shift
		[[ "$var" = "$val" ]] && continue
		val="${val#*=}"
		json_add_string "$var" "$val"
	done
}

_wireless_add_vif() {
	local name="$1"; shift
	local ifname="$1"; shift

	_wdev_notify_init $CMD_SET_DATA "interface" "$name"
	json_add_string "ifname" "$ifname"
	_wdev_add_variables "$@"
	_wdev_notify
}

_wireless_add_vlan() {
	local name="$1"; shift
	local ifname="$1"; shift

	_wdev_notify_init $CMD_SET_DATA interface "$__cur_interface" "vlan" "$name"
	json_add_string "ifname" "$ifname"
	_wdev_add_variables "$@"
	_wdev_notify
}

_wireless_set_up() {
	_wdev_notify_init $CMD_UP
	_wdev_notify
}

_wireless_set_data() {
	_wdev_notify_init $CMD_SET_DATA
	_wdev_add_variables "$@"
	_wdev_notify
}

_wireless_add_process() {
	_wdev_notify_init $CMD_PROCESS_ADD
	local exe="$2"
	[ -L "$exe" ] && exe="$(readlink -f "$exe")"
	json_add_int pid "$1"
	json_add_string exe "$exe"
	[ -n "$3" ] && json_add_boolean required 1
	[ -n "$4" ] && json_add_boolean keep 1
	exe2="$(readlink -f /proc/$1/exe)"
	[ "$exe" != "$exe2" ] && echo "WARNING (wireless_add_process): executable path $exe does not match process $1 path ($exe2)"
	_wdev_notify
}

_wireless_process_kill_all() {
	_wdev_notify_init $CMD_PROCESS_KILL_ALL
	[ -n "$1" ] && json_add_int signal "$1"
	_wdev_notify
}

_wireless_set_retry() {
	_wdev_notify_init $CMD_SET_RETRY
	json_add_int retry "$1"
	_wdev_notify
}

_wdev_wrapper \
	wireless_add_vif \
	wireless_add_vlan \
	wireless_set_up \
	wireless_set_data \
	wireless_add_process \
	wireless_process_kill_all \
	wireless_set_retry \

wireless_vif_parse_encryption() {
	json_get_vars encryption rsn_override
	set_default encryption none

	set_default rsn_override 1
	auth_mode_open=1
	auth_mode_shared=0
	auth_type=none
	wpa_override_cipher=
	rsn_override_pairwise=

	if [ "$hwmode" = "ad" ]; then
		wpa_cipher="GCMP"
	else
		wpa_cipher="CCMP"
		case "$encryption" in
			sae*|wpa3*|psk3*|owe)
				if [ "$rsn_override" -gt 0 ]; then
					wpa_override_cipher="${wpa3_cipher}$wpa_cipher"
				else
					wpa_cipher="${wpa3_cipher}$wpa_cipher"
				fi
			;;
		esac
	fi

	case "$encryption" in
		*tkip+aes|*tkip+ccmp|*aes+tkip|*ccmp+tkip) wpa_cipher="CCMP TKIP";;
		*ccmp256) wpa_cipher="CCMP-256";;
		*aes|*ccmp) wpa_cipher="CCMP";;
		*tkip) wpa_cipher="TKIP";;
		*gcmp256) wpa_cipher="GCMP-256";;
		*gcmp) wpa_cipher="GCMP";;
		wpa3-192*) wpa_cipher="GCMP-256";;
		*) rsn_override_pairwise="$wpa_override_cipher";;
	esac

	# 802.11n requires CCMP for WPA
	[ "$enable_ht:$wpa_cipher" = "1:TKIP" ] && wpa_cipher="CCMP TKIP"

	# Examples:
	# psk-mixed/tkip    => WPA1+2 PSK, TKIP
	# wpa-psk2/tkip+aes => WPA2 PSK, CCMP+TKIP
	# wpa2/tkip+aes     => WPA2 RADIUS, CCMP+TKIP

	case "$encryption" in
		wpa2*|wpa3*|*psk2*|psk3*|sae*|owe*|dpp)
			wpa=2
		;;
		wpa*mixed*|*psk*mixed*)
			wpa=3
		;;
		wpa*|*psk*)
			wpa=1
		;;
		*)
			wpa=0
			wpa_cipher=
		;;
	esac
	wpa_pairwise="$wpa_cipher"

	case "$encryption" in
		owe*)
			auth_type=owe
		;;
		dpp)
			auth_type=dpp
		;;
		wpa3-192*)
			auth_type=eap192
		;;
		wpa3-mixed*)
			auth_type=eap-eap2
		;;
		wpa3*)
			auth_type=eap2
		;;
		psk3-mixed*|sae-mixed*)
			auth_type=psk-sae
		;;
		psk3*|sae*)
			auth_type=sae
		;;
		*psk*)
			auth_type=psk
		;;
		*wpa*|*8021x*)
			auth_type=eap
		;;
		*wep*)
			auth_type=wep
			case "$encryption" in
				*shared*)
					auth_mode_open=0
					auth_mode_shared=1
				;;
				*mixed*)
					auth_mode_shared=1
				;;
			esac
		;;
	esac
}

_wireless_set_brsnoop_isolation() {
	local multicast_to_unicast="$1"
	local isolate

	json_get_vars isolate proxy_arp

	[ ${isolate:-0} -gt 0 -o -z "$network_bridge" ] && return
	[ ${multicast_to_unicast:-1} -gt 0 -o ${proxy_arp:-0} -gt 0 ] && json_add_boolean isolate 1
}

for_each_interface() {
	local _w_types="$1"; shift
	local _w_ifaces _w_iface
	local _w_type
	local _w_found

	local multicast_to_unicast

	json_get_keys _w_ifaces interfaces
	json_select interfaces
	for _w_iface in $_w_ifaces; do
		json_select "$_w_iface"
		if [ -n "$_w_types" ]; then
			json_get_var network_bridge bridge
			json_get_var network_ifname bridge-ifname
			json_get_var multicast_to_unicast multicast_to_unicast
			json_select config
			_wireless_set_brsnoop_isolation "$multicast_to_unicast"
			json_get_var _w_type mode
			json_select ..
			_w_types=" $_w_types "
			[[ "${_w_types%$_w_type*}" = "$_w_types" ]] && {
				json_select ..
				continue
			}
		fi
		__cur_interface="$_w_iface"
		"$@" "$_w_iface"
		json_select ..
	done
	json_select ..
}

for_each_vlan() {
	local _w_vlans _w_vlan

	json_get_keys _w_vlans vlans
	json_select vlans
	for _w_vlan in $_w_vlans; do
		json_select "$_w_vlan"
		json_select config
		"$@" "$_w_vlan"
		json_select ..
		json_select ..
	done
	json_select ..
}

for_each_station() {
	local _w_stas _w_sta

	json_get_keys _w_stas stas
	json_select stas
	for _w_sta in $_w_stas; do
		json_select "$_w_sta"
		json_select config
		"$@" "$_w_sta"
		json_select ..
		json_select ..
	done
	json_select ..
}

_wdev_common_device_config() {
	config_add_string channel hwmode band htmode noscan
}

_wdev_common_iface_config() {
	config_add_string mode ssid encryption 'key:wpakey'
	config_add_boolean bridge_isolate
	config_add_array tags
}

_wdev_common_vlan_config() {
	config_add_string name vid iface
	config_add_boolean bridge_isolate
}

_wdev_common_station_config() {
	config_add_string key vid iface
	config_add_array mac
}

init_wireless_driver() {
	name="$1"; shift
	cmd="$1"; shift

	case "$cmd" in
		dump)
			add_driver() {
				eval "drv_$1_cleanup"

				json_init
				json_add_string name "$1"

				json_add_array device
				_wdev_common_device_config
				eval "drv_$1_init_device_config"
				json_close_array

				json_add_array iface
				_wdev_common_iface_config
				eval "drv_$1_init_iface_config"
				json_close_array

				json_add_array vlan
				_wdev_common_vlan_config
				eval "drv_$1_init_vlan_config"
				json_close_array

				json_add_array station
				_wdev_common_station_config
				eval "drv_$1_init_station_config"
				json_close_array

				json_dump
			}
		;;
		setup|teardown)
			interface="$1"; shift
			data="$1"; shift
			export __netifd_device="$interface"

			add_driver() {
				[[ "$name" == "$1" ]] || return 0
				_wdev_handler "$1" "$cmd"
			}
		;;
	esac
}
