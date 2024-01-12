#!/bin/sh /etc/rc.common
# Copyright (C) 2013 OpenWrt.org

# start after and stop before networking
START=12
STOP=89
PIDCOUNT=0

USE_PROCD=1
PROG=/sbin/logread

validate_log_section()
{
	uci_load_validate system system "$1" "$2" \
		'log_file:string' \
		'log_size:uinteger' \
		'log_hostname:string' \
		'log_ip:host' \
		'log_remote:bool:1' \
		'log_port:port:514' \
		'log_proto:or("tcp", "udp"):udp' \
		'log_trailer_null:bool:0' \
		'log_prefix:string'
}

validate_log_daemon()
{
	uci_load_validate system system "$1" "$2" \
		'log_size:uinteger:0' \
		'log_buffer_size:uinteger:0'
}

start_service_daemon()
{
	[ $log_buffer_size -eq 0 -a $log_size -gt 0 ] && log_buffer_size=$log_size
	[ $log_buffer_size -eq 0 ] && log_buffer_size=64
	procd_open_instance logd
	procd_set_param command "/sbin/logd"
	procd_append_param command -S "${log_buffer_size}"
	procd_set_param respawn 5 1 -1
	procd_close_instance
}

start_service_file()
{
	PIDCOUNT="$(( ${PIDCOUNT} + 1))"
	local pid_file="/var/run/logread.${PIDCOUNT}.pid"

	[ "$2" = 0 ] || {
		echo "validation failed"
		return 1
	}
	[ -z "${log_file}" ] && return

	[ "$_BOOT" = "1" ] &&
		[ "$(procd_get_mountpoints "${log_file}")" ] && return 0

	mkdir -p "$(dirname "${log_file}")"

	procd_open_instance logfile
	procd_set_param command "$PROG" -f -F "$log_file" -p "$pid_file"
	[ -n "${log_size}" ] && procd_append_param command -S "$log_size"
	procd_close_instance
}

start_service_remote()
{
	PIDCOUNT="$(( ${PIDCOUNT} + 1))"
	local pid_file="/var/run/logread.${PIDCOUNT}.pid"

	[ "$2" = 0 ] || {
		echo "validation failed"
		return 1
	}
	[ "${log_remote}" -ne 0 ] || return
	[ -z "${log_ip}" ] && return
	[ -z "${log_hostname}" ] && log_hostname=$(cat /proc/sys/kernel/hostname)

	procd_open_instance logremote
	procd_set_param command "$PROG" -f -h "$log_hostname" -r "$log_ip" "${log_port}" -p "$pid_file"
	case "${log_proto}" in
		"udp") procd_append_param command -u;;
		"tcp") [ "${log_trailer_null}" -eq 1 ] && procd_append_param command -0;;
	esac
	[ -z "${log_prefix}" ] || procd_append_param command -P "${log_prefix}"
	procd_close_instance
}

register_mount_trigger()
{
	[ -n "${log_file}" ] && procd_add_action_mount_trigger start "${log_file}"
}

service_triggers()
{
	config_load system
	procd_add_reload_trigger "system"
	procd_add_validation validate_log_section
	config_foreach validate_log_section system register_mount_trigger
}

start_service()
{
	config_load system
	config_foreach validate_log_daemon system start_service_daemon
	config_foreach validate_log_section system start_service_file
	config_foreach validate_log_section system start_service_remote
}

boot() {
	_BOOT=1 start
}
