. /lib/functions.sh
. /usr/share/libubox/jshn.sh

json_select_array() {
	local _json_no_warning=1

	json_select "$1"
	[ $? = 0 ] && return

	json_add_array "$1"
	json_close_array

	json_select "$1"
}

json_select_object() {
	local _json_no_warning=1

	json_select "$1"
	[ $? = 0 ] && return

	json_add_object "$1"
	json_close_object

	json_select "$1"
}

ucidef_set_interface() {
	local network=$1; shift

	[ -z "$network" ] && return

	json_select_object network
	json_select_object "$network"

	while [ -n "$1" ]; do
		local opt=$1; shift
		local val=$1; shift

		[ -n "$opt" -a -n "$val" ] || break

		[ "$opt" = "device" -a "$val" != "${val/ //}" ] && {
			json_select_array "ports"
			for e in $val; do json_add_string "" "$e"; done
			json_close_array
		} || {
			json_add_string "$opt" "$val"
		}
	done

	if ! json_is_a protocol string; then
		case "$network" in
			lan) json_add_string protocol static ;;
			wan) json_add_string protocol dhcp ;;
			*) json_add_string protocol none ;;
		esac
	fi

	json_select ..
	json_select ..
}

ucidef_set_board_id() {
	json_select_object model
	json_add_string id "$1"
	json_select ..
}

ucidef_set_model_name() {
	json_select_object model
	json_add_string name "$1"
	json_select ..
}

ucidef_set_compat_version() {
	json_select_object system
	json_add_string compat_version "${1:-1.0}"
	json_select ..
}

ucidef_set_interface_lan() {
	ucidef_set_interface "lan" device "$1" protocol "${2:-static}"
}

ucidef_set_interface_wan() {
	ucidef_set_interface "wan" device "$1" protocol "${2:-dhcp}"
}

ucidef_set_interfaces_lan_wan() {
	local lan_if="$1"
	local wan_if="$2"

	ucidef_set_interface_lan "$lan_if"
	ucidef_set_interface_wan "$wan_if"
}

ucidef_set_bridge_device() {
	json_select_object bridge
	json_add_string name "${1:-switch0}"
	json_select ..
}

ucidef_set_bridge_mac() {
	json_select_object bridge
	json_add_string macaddr "${1}"
	json_select ..
}

ucidef_set_network_device_mac() {
	json_select_object "network_device"
	json_select_object "${1}"
	json_add_string macaddr "${2}"
	json_select ..
	json_select ..
}

ucidef_set_network_device_path() {
	json_select_object "network_device"
	json_select_object "$1"
	json_add_string path "$2"
	json_select ..
	json_select ..
}

_ucidef_add_switch_port() {
	# inherited: $num $device $need_tag $want_untag $role $index $prev_role
	# inherited: $n_cpu $n_ports $n_vlan $cpu0 $cpu1 $cpu2 $cpu3 $cpu4 $cpu5

	n_ports=$((n_ports + 1))

	json_select_array ports
		json_add_object
			json_add_int num "$num"
			[ -n "$device"     ] && json_add_string  device     "$device"
			[ -n "$need_tag"   ] && json_add_boolean need_tag   "$need_tag"
			[ -n "$want_untag" ] && json_add_boolean want_untag "$want_untag"
			[ -n "$role"       ] && json_add_string  role       "$role"
			[ -n "$index"      ] && json_add_int     index      "$index"
		json_close_object
	json_select ..

	# record pointer to cpu entry for lookup in _ucidef_finish_switch_roles()
	[ -n "$device" ] && {
		export "cpu$n_cpu=$n_ports"
		n_cpu=$((n_cpu + 1))
	}

	# create/append object to role list
	[ -n "$role" ] && {
		json_select_array roles

		if [ "$role" != "$prev_role" ]; then
			json_add_object
				json_add_string role "$role"
				json_add_string ports "$num"
			json_close_object

			prev_role="$role"
			n_vlan=$((n_vlan + 1))
		else
			json_select_object "$n_vlan"
				json_get_var port ports
				json_add_string ports "$port $num"
			json_select ..
		fi

		json_select ..
	}
}

_ucidef_finish_switch_roles() {
	# inherited: $name $n_cpu $n_vlan $cpu0 $cpu1 $cpu2 $cpu3 $cpu4 $cpu5
	local index role roles num device need_tag want_untag port ports

	json_select switch
		json_select "$name"
			json_get_keys roles roles
		json_select ..
	json_select ..

	for index in $roles; do
		eval "port=\$cpu$(((index - 1) % n_cpu))"

		json_select switch
			json_select "$name"
				json_select ports
					json_select "$port"
						json_get_vars num device need_tag want_untag
					json_select ..
				json_select ..

				if [ ${need_tag:-0} -eq 1 -o ${want_untag:-0} -ne 1 ]; then
					num="${num}t"
					device="${device}.${index}"
				fi

				json_select roles
					json_select "$index"
						json_get_vars role ports
						json_add_string ports "$ports $num"
						json_add_string device "$device"
					json_select ..
				json_select ..
			json_select ..
		json_select ..

		json_select_object network
			local devices

			json_select_object "$role"
				# attach previous interfaces (for multi-switch devices)
				json_get_var devices device
				if ! list_contains devices "$device"; then
					devices="${devices:+$devices }$device"
				fi
			json_select ..
		json_select ..

		ucidef_set_interface "$role" device "$devices"
	done
}

ucidef_set_ar8xxx_switch_mib() {
	local name="$1"
	local type="$2"
	local interval="$3"

	json_select_object switch
		json_select_object "$name"
			json_add_int ar8xxx_mib_type $type
			json_add_int ar8xxx_mib_poll_interval $interval
		json_select ..
	json_select ..
}

ucidef_add_switch() {
	local name="$1"; shift
	local port num role device index need_tag prev_role
	local cpu0 cpu1 cpu2 cpu3 cpu4 cpu5
	local n_cpu=0 n_vlan=0 n_ports=0

	json_select_object switch
		json_select_object "$name"
			json_add_boolean enable 1
			json_add_boolean reset 1

			for port in "$@"; do
				case "$port" in
					[0-9]*@*)
						num="${port%%@*}"
						device="${port##*@}"
						need_tag=0
						want_untag=0
						[ "${num%t}" != "$num" ] && {
							num="${num%t}"
							need_tag=1
						}
						[ "${num%u}" != "$num" ] && {
							num="${num%u}"
							want_untag=1
						}
					;;
					[0-9]*:*:[0-9]*)
						num="${port%%:*}"
						index="${port##*:}"
						role="${port#[0-9]*:}"; role="${role%:*}"
					;;
					[0-9]*:*)
						num="${port%%:*}"
						role="${port##*:}"
					;;
				esac

				if [ -n "$num" ] && [ -n "$device$role" ]; then
					_ucidef_add_switch_port
				fi

				unset num device role index need_tag want_untag
			done
		json_select ..
	json_select ..

	_ucidef_finish_switch_roles
}

ucidef_add_switch_attr() {
	local name="$1"
	local key="$2"
	local val="$3"

	json_select_object switch
		json_select_object "$name"

		case "$val" in
			true|false) [ "$val" != "true" ]; json_add_boolean "$key" $? ;;
			[0-9]) json_add_int "$key" "$val" ;;
			*) json_add_string "$key" "$val" ;;
		esac

		json_select ..
	json_select ..
}

ucidef_add_switch_port_attr() {
	local name="$1"
	local port="$2"
	local key="$3"
	local val="$4"
	local ports i num

	json_select_object switch
	json_select_object "$name"

	json_get_keys ports ports
	json_select_array ports

	for i in $ports; do
		json_select "$i"
		json_get_var num num

		if [ -n "$num" ] && [ $num -eq $port ]; then
			json_select_object attr

			case "$val" in
				true|false) [ "$val" != "true" ]; json_add_boolean "$key" $? ;;
				[0-9]) json_add_int "$key" "$val" ;;
				*) json_add_string "$key" "$val" ;;
			esac

			json_select ..
		fi

		json_select ..
	done

	json_select ..
	json_select ..
	json_select ..
}

ucidef_set_interface_macaddr() {
	local network="$1"
	local macaddr="$2"

	ucidef_set_interface "$network" macaddr "$macaddr"
}

ucidef_set_label_macaddr() {
	local macaddr="$1"

	json_select_object system
		json_add_string label_macaddr "$macaddr"
	json_select ..
}

ucidef_add_atm_bridge() {
	local vpi="$1"
	local vci="$2"
	local encaps="$3"
	local payload="$4"
	local nameprefix="$5"

	json_select_object dsl
		json_select_object atmbridge
			json_add_int vpi "$vpi"
			json_add_int vci "$vci"
			json_add_string encaps "$encaps"
			json_add_string payload "$payload"
			json_add_string nameprefix "$nameprefix"
		json_select ..
	json_select ..
}

ucidef_add_adsl_modem() {
	local annex="$1"
	local firmware="$2"

	json_select_object dsl
		json_select_object modem
			json_add_string type "adsl"
			json_add_string annex "$annex"
			json_add_string firmware "$firmware"
		json_select ..
	json_select ..
}

ucidef_add_vdsl_modem() {
	local annex="$1"
	local tone="$2"
	local xfer_mode="$3"

	json_select_object dsl
		json_select_object modem
			json_add_string type "vdsl"
			json_add_string annex "$annex"
			json_add_string tone "$tone"
			json_add_string xfer_mode "$xfer_mode"
		json_select ..
	json_select ..
}

ucidef_set_led_ataport() {
	_ucidef_set_led_trigger "$1" "$2" "$3" ata"$4"
}

_ucidef_set_led_common() {
	local cfg="led_$1"
	local name="$2"
	local sysfs="$3"

	json_select_object led

	json_select_object "$1"
	json_add_string name "$name"
	json_add_string sysfs "$sysfs"
}

ucidef_set_led_default() {
	local default="$4"

	_ucidef_set_led_common "$1" "$2" "$3"

	json_add_string default "$default"
	json_select ..

	json_select ..
}

ucidef_set_led_heartbeat() {
	_ucidef_set_led_common "$1" "$2" "$3"

	json_add_string trigger heartbeat
	json_select ..

	json_select ..
}

ucidef_set_led_gpio() {
	local gpio="$4"
	local inverted="$5"

	_ucidef_set_led_common "$1" "$2" "$3"

	json_add_string trigger "$trigger"
	json_add_string type gpio
	json_add_int gpio "$gpio"
	json_add_boolean inverted "$inverted"
	json_select ..

	json_select ..
}

ucidef_set_led_ide() {
	_ucidef_set_led_trigger "$1" "$2" "$3" disk-activity
}

ucidef_set_led_netdev() {
	local dev="$4"
	local mode="${5:-link tx rx}"

	_ucidef_set_led_common "$1" "$2" "$3"

	json_add_string type netdev
	json_add_string device "$dev"
	json_add_string mode "$mode"
	json_select ..

	json_select ..
}

ucidef_set_led_oneshot() {
	_ucidef_set_led_timer $1 $2 $3 "oneshot" $4 $5
}

ucidef_set_led_portstate() {
	local port_state="$4"

	_ucidef_set_led_common "$1" "$2" "$3"

	json_add_string trigger port_state
	json_add_string type portstate
	json_add_string port_state "$port_state"
	json_select ..

	json_select ..
}

ucidef_set_led_rssi() {
	local iface="$4"
	local minq="$5"
	local maxq="$6"
	local offset="${7:-0}"
	local factor="${8:-1}"

	_ucidef_set_led_common "$1" "$2" "$3"

	json_add_string type rssi
	json_add_string name "$name"
	json_add_string iface "$iface"
	json_add_string minq "$minq"
	json_add_string maxq "$maxq"
	json_add_string offset "$offset"
	json_add_string factor "$factor"
	json_select ..

	json_select ..
}

ucidef_set_led_switch() {
	local trigger_name="$4"
	local port_mask="$5"
	local speed_mask="$6"
	local mode="$7"

	_ucidef_set_led_common "$1" "$2" "$3"

	json_add_string trigger "$trigger_name"
	json_add_string type switch
	json_add_string mode "$mode"
	json_add_string port_mask "$port_mask"
	json_add_string speed_mask "$speed_mask"
	json_select ..

	json_select ..
}

_ucidef_set_led_timer() {
	local trigger_name="$4"
	local delayon="$5"
	local delayoff="$6"

	_ucidef_set_led_common "$1" "$2" "$3"

	json_add_string type "$trigger_name"
	json_add_string trigger "$trigger_name"
	json_add_int delayon "$delayon"
	json_add_int delayoff "$delayoff"
	json_select ..

	json_select ..
}

ucidef_set_led_timer() {
	_ucidef_set_led_timer $1 $2 $3 "timer" $4 $5
}

_ucidef_set_led_trigger() {
	local trigger_name="$4"

	_ucidef_set_led_common "$1" "$2" "$3"

	json_add_string trigger "$trigger_name"
	json_select ..

	json_select ..
}

ucidef_set_led_usbdev() {
	local dev="$4"

	_ucidef_set_led_common "$1" "$2" "$3"

	json_add_string type usb
	json_add_string device "$dev"
	json_select ..

	json_select ..
}

ucidef_set_led_usbhost() {
	_ucidef_set_led_trigger "$1" "$2" "$3" usb-host
}

ucidef_set_led_usbport() {
	local obj="$1"
	local name="$2"
	local sysfs="$3"
	shift
	shift
	shift

	_ucidef_set_led_common "$obj" "$name" "$sysfs"

	json_add_string type usbport
	json_select_array ports
		for port in "$@"; do
			json_add_string port "$port"
		done
	json_select ..
	json_select ..

	json_select ..
}

ucidef_set_led_wlan() {
	_ucidef_set_led_trigger "$1" "$2" "$3" "$4"
}

ucidef_set_rssimon() {
	local dev="$1"
	local refresh="$2"
	local threshold="$3"

	json_select_object rssimon

	json_select_object "$dev"
	[ -n "$refresh" ] && json_add_int refresh "$refresh"
	[ -n "$threshold" ] && json_add_int threshold "$threshold"
	json_select ..

	json_select ..
}

ucidef_add_gpio_switch() {
	local cfg="$1"
	local name="$2"
	local pin="$3"
	local default="${4:-0}"

	json_select_object gpioswitch
		json_select_object "$cfg"
			json_add_string name "$name"
			json_add_string pin "$pin"
			json_add_int default "$default"
		json_select ..
	json_select ..
}

ucidef_set_hostname() {
	local hostname="$1"

	json_select_object system
		json_add_string hostname "$hostname"
	json_select ..
}

ucidef_set_ntpserver() {
	local server

	json_select_object system
		json_select_array ntpserver
			for server in "$@"; do
				json_add_string "" "$server"
			done
		json_select ..
	json_select ..
}

ucidef_add_wlan() {
	local path="$1"; shift

	ucidef_wlan_idx=${ucidef_wlan_idx:-0}

	json_select_object wlan
	json_select_object "wl$ucidef_wlan_idx"
	json_add_string path "$path"
	json_add_fields "$@"
	json_select ..
	json_select ..

	ucidef_wlan_idx="$((ucidef_wlan_idx + 1))"
}

board_config_update() {
	json_init
	[ -f ${CFG} ] && json_load "$(cat ${CFG})"

	# auto-initialize model id and name if applicable
	if ! json_is_a model object; then
		json_select_object model
			[ -f "/tmp/sysinfo/board_name" ] && \
				json_add_string id "$(cat /tmp/sysinfo/board_name)"
			[ -f "/tmp/sysinfo/model" ] && \
				json_add_string name "$(cat /tmp/sysinfo/model)"
		json_select ..
	fi
}

board_config_flush() {
	json_dump -i -o ${CFG}
}
