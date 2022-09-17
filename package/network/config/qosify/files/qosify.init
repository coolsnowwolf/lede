#!/bin/sh /etc/rc.common
# Copyright (c) 2021 OpenWrt.org

START=19

USE_PROCD=1
PROG=/usr/sbin/qosify

add_option() {
	local type="$1"
	local name="$2"

	config_get val "$cfg" "$name"

	[ -n "$val" ] && json_add_$type "$name" "$val"
}

add_flow_config() {
	local cfg="$1"

	add_option string dscp_prio
	add_option string dscp_bulk
	add_option int bulk_trigger_timeout
	add_option int bulk_trigger_pps
	add_option int prio_max_avg_pkt_len
}

add_defaults() {
	cfg="$1"

	json_add_boolean reset 1

	config_get files "$cfg" defaults
	json_add_array files
	for i in $files; do
		json_add_string "" "$i"
	done
	json_close_array

	add_flow_config "$cfg"
	add_option int timeout
	add_option string dscp_icmp
	add_option string dscp_default_udp
	add_option string dscp_default_tcp
}

add_interface() {
	local cfg="$1"

	config_get_bool disabled "$cfg" disabled 0
	[ "$disabled" -gt 0 ] && return

	config_get name "$cfg" name
	json_add_object "$name"

	config_get bw "$cfg" bandwidth

	config_get bw_up "$cfg" bandwidth_up
	bw_up="${bw_up:-$bw}"
	[ -n "$bw_up" ] && json_add_string bandwidth_up "$bw_up"

	config_get bw_down "$cfg" bandwidth_down
	bw_down="${bw_down:-$bw}"
	[ -n "$bw_down" ] && json_add_string bandwidth_down "$bw_down"

	add_option string bandwidth
	add_option boolean ingress
	add_option boolean egress
	add_option string mode
	add_option boolean nat
	add_option boolean host_isolate
	add_option boolean autorate_ingress
	add_option string ingress_options
	add_option string egress_options

	config_get user_options "$cfg" options

	config_get otype "$cfg" overhead_type
	options=
	case "$otype" in
		none);;
		manual)
			config_get overhead "$cfg" overhead
			[ -n "$overhead" ] && append options "overhead $overhead"

			config_get encap "$cfg" overhead_encap
			[ -n "$encap" ] && append options "$encap"
		;;
		conservative|\
		pppoa-vcmux|\
		pppoa-llc|\
		pppoe-vcmux|\
		pppoe-llcsnap|\
		bridged-vcmux|\
		bridged-llcsnap|\
		ipoa-vcmux|\
		ipoa-llcsnap|\
		pppoe-ptm|\
		bridged-ptm|\
		docsis|\
		ethernet)
			append options "$otype"
		;;
	esac

	config_get mpu "$cfg" overhead_mpu
	[ -n "$mpu" ] && append options "mpu $mpu"

	config_get ovlan "$cfg" overhead_vlan
	[ "${ovlan:-0}" -ge 2 ] && append options "ether-vlan"
	[ "${ovlan:-0}" -ge 1 ] && append options "ether-vlan"

	[ -n "$user_options" ] && append options "$user_options"
	[ -n "$options" ] && json_add_string options "$options"

	json_close_object
}

add_class() {
	local cfg="$1"

	config_get value "$cfg" value
	config_get ingress "$cfg" ingress
	config_get egress "$cfg" egress

	json_add_object "$cfg"
	json_add_string ingress "${ingress:-$value}"
	json_add_string egress "${egress:-$value}"
	add_flow_config "$cfg"
	json_close_object
}


reload_service() {
	json_init

	config_load qosify

	config_foreach add_defaults defaults

	json_add_object interfaces
	config_foreach add_interface interface
	json_close_object

	json_add_object classes
	config_foreach add_class class
	config_foreach add_class alias
	json_close_object

	json_add_object devices
	config_foreach add_interface device
	json_close_object

	ubus call qosify config "$(json_dump)"
}

service_triggers() {
	procd_add_reload_trigger qosify
}

start_service() {
	procd_open_instance
	procd_set_param command "$PROG"
	procd_set_param respawn
	procd_close_instance
}

service_started() {
	ubus -t 10 wait_for qosify
	[ $? = 0 ] && reload_service
}
