#!/bin/sh
#
# Copyright © 2017 OpenWrt.org
#

. /lib/functions.sh
. /lib/functions/leds.sh
. /usr/share/libubox/jshn.sh

preinit_match_diag_led() {
	local CFG keys key cfg name sysfs default

	CFG=/etc/board.json
	if [ ! -s $CFG ]; then
		CFG=/tmp/board.json
		[ -s /tmp/sysinfo/model ] || return
		/bin/board_detect $CFG || return
	fi

	json_init
	json_load "$(cat $CFG)"
	json_get_keys keys led
	json_is_a led object || return

	json_select led
	for key in $keys; do
		json_select "$key"
		json_get_vars name sysfs default

		if [ "$name" = "DIAG" -a "$default" = "1" ]; then
			status_led="$sysfs"
			return
		fi
		json_select ..
	done
}

match_diag_led() {
	local name
	local default
	local sysfs
	config_get name "$1" name
	config_get default "$1" default
	config_get sysfs "$1" sysfs

	if [ "$name" = "DIAG" -a "$default" = "1" ]; then
		status_led="$sysfs"
	fi
}

get_status_led() {
	if [ -s /etc/config/system ]; then
		config_load system
		config_foreach match_diag_led led
	else
		preinit_match_diag_led
	fi
}

set_state() {
	get_status_led

	case "$1" in
	preinit)
		status_led_blink_preinit
		;;

	failsafe)
		status_led_blink_failsafe
		;;

	preinit_regular)
		status_led_blink_preinit_regular
		;;

	done)
		status_led_on
		;;
	esac
}
