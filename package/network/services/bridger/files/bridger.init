#!/bin/sh /etc/rc.common
# Copyright (c) 2021 OpenWrt.org

START=19

USE_PROCD=1
PROG=/usr/sbin/bridger

add_blacklist() {
	cfg="$1"

	config_get blacklist "$cfg" blacklist
	for i in $blacklist; do
		json_add_string "" "$i"
	done
}

reload_service() {
	config_load bridger

	json_init
	json_add_string name "config"
	json_add_array devices
	config_foreach add_blacklist defaults
	json_close_array

	ubus call bridger set_blacklist "$(json_dump)"
}

service_triggers() {
	procd_add_reload_trigger bridger
}

start_service() {
	procd_open_instance
	procd_set_param command "$PROG"
	procd_set_param respawn
	procd_close_instance
}

service_started() {
	ubus -t 10 wait_for bridger
	[ $? = 0 ] && reload_service
}
