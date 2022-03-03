#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-only

. /lib/functions/leds.sh

get_status_led() {
	local status_led_file

	status_led_file=$(find /sys/class/leds/ -name "*:power" | head -n1)
	if [ -d "$status_led_file" ]; then
		basename $status_led_file
		return
	fi;
}

get_failsafe_led() {
	local status_led_file

	status_led_file=$(find /sys/class/leds/ -name "*:fault" | head -n1)
	if [ -d "$status_led_file" ]; then
		basename $status_led_file
		return
	fi;
}

set_state() {
	status_led=$(get_status_led)

	[ -z "$status_led" ] && return

	case "$1" in
	preinit)
		status_led_blink_preinit
		;;
	failsafe)
		status_led_off
		status_led=$(get_failsafe_led)
		status_led_blink_failsafe
		;;
	preinit_regular)
		status_led_blink_preinit_regular
		;;
        upgrade)
                status_led_blink_preinit_regular
                ;;
	done)
		status_led_on
		;;
	esac
}
