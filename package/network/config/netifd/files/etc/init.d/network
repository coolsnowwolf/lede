#!/bin/sh /etc/rc.common

START=20
STOP=90

USE_PROCD=1

init_switch() {
	setup_switch() { return 0; }

	include /lib/network
	setup_switch
}

start_service() {
	init_switch

	procd_open_instance
	procd_set_param command /sbin/netifd
	procd_set_param respawn
	procd_set_param watch network.interface
	[ -e /proc/sys/kernel/core_pattern ] && {
		procd_set_param limits core="unlimited"
	}
	procd_close_instance
}

reload_service() {
	local rv=0

	init_switch
	ubus call network reload || rv=1
	/sbin/wifi reload_legacy
	return $rv
}

stop_service() {
	/sbin/wifi down
	ifdown -a
	sleep 1
}

validate_atm_bridge_section()
{
	uci_validate_section network "atm-bridge" "${1}" \
		'unit:uinteger:0' \
		'vci:range(32, 65535):35' \
		'vpi:range(0, 255):8' \
		'atmdev:uinteger:0' \
		'encaps:or("llc", "vc"):llc' \
		'payload:or("bridged", "routed"):bridged'
}

validate_route_section()
{
	uci_validate_section network route "${1}" \
		'interface:string' \
		'target:cidr4' \
		'netmask:netmask4' \
		'gateway:ip4addr' \
		'metric:uinteger' \
		'mtu:uinteger' \
		'table:or(range(0,65535),string)'
}

validate_route6_section()
{
	uci_validate_section network route6 "${1}" \
		'interface:string' \
		'target:cidr6' \
		'gateway:ip6addr' \
		'metric:uinteger' \
		'mtu:uinteger' \
		'table:or(range(0,65535),string)'
}

validate_rule_section()
{
	uci_validate_section network rule "${1}" \
		'in:string' \
		'out:string' \
		'src:cidr4' \
		'dest:cidr4' \
		'tos:range(0,31)' \
		'mark:string' \
		'invert:bool' \
		'lookup:or(range(0,65535),string)' \
		'goto:range(0,65535)' \
		'action:or("prohibit", "unreachable", "blackhole", "throw")'
}

validate_rule6_section()
{
	uci_validate_section network rule6 "${1}" \
		'in:string' \
		'out:string' \
		'src:cidr6' \
		'dest:cidr6' \
		'tos:range(0,31)' \
		'mark:string' \
		'invert:bool' \
		'lookup:or(range(0,65535),string)' \
		'goto:range(0,65535)' \
		'action:or("prohibit", "unreachable", "blackhole", "throw")'
}

validate_switch_section()
{
	uci_validate_section network switch "${1}" \
		'name:string' \
		'enable:bool' \
		'enable_vlan:bool' \
		'reset:bool' \
		'ar8xxx_mib_poll_interval:uinteger' \
		'ar8xxx_mib_type:range(0,1)'
}

validate_switch_vlan()
{
	uci_validate_section network switch_vlan "${1}" \
		'device:string' \
		'vlan:uinteger' \
		'ports:list(ports)'
}

service_triggers()
{
	procd_add_reload_trigger network wireless

	procd_open_validate
	validate_atm_bridge_section
	validate_route_section
	[ -e /proc/sys/net/ipv6 ] && validate_route6_section
	validate_rule_section
	[ -e /proc/sys/net/ipv6 ] && validate_rule6_section
	validate_switch_section
	validate_switch_vlan
	procd_close_validate
}

shutdown() {
	ifdown -a
	sleep 1
}
