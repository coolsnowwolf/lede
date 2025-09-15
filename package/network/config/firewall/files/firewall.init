#!/bin/sh /etc/rc.common

START=19
USE_PROCD=1
QUIET=""

LOCK_FILE="/var/lock/procd_firewall.lock"
set_lock() {
	exec 1000>"$LOCK_FILE"
	flock -xn 1000
}

unset_lock() {
	flock -u 1000
	rm -rf "$LOCK_FILE"
}

validate_firewall_redirect()
{
	uci_validate_section firewall redirect "${1}" \
		'proto:or(uinteger, string)' \
		'src:string' \
		'src_ip:cidr' \
		'src_dport:or(port, portrange)' \
		'dest:string' \
		'dest_ip:cidr' \
		'dest_port:or(port, portrange)' \
		'target:or("SNAT", "DNAT")'
}

validate_firewall_rule()
{
	uci_validate_section firewall rule "${1}" \
		'proto:or(uinteger, string)' \
		'src:string' \
		'dest:string' \
		'src_port:or(port, portrange)' \
		'dest_port:or(port, portrange)' \
		'target:string'
}

service_triggers() {
	procd_add_reload_trigger firewall	
	set_lock

	procd_open_validate
	validate_firewall_redirect
	validate_firewall_rule
	procd_close_validate
	unset_lock
}

restart() {
	unset_lock
	set_lock
	fw3 restart
	unset_lock
}

start_service() {
	set_lock
	fw3 ${QUIET} start
	unset_lock
}

stop_service() {
	fw3 flush
	unset_lock
}

reload_service() {
  unset_lock
  set_lock
	reload_config firewall
	fw3 reload
	unset_lock
}

boot() {
	# Be silent on boot, firewall might be started by hotplug already,
	# so don't complain in syslog.
	QUIET=-q
	start
}
