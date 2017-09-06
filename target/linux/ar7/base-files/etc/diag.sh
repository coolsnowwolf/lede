#!/bin/sh
# Copyright (C) 2007-2013 OpenWrt.org

. /lib/functions/leds.sh

get_status_led() {
	[ -d "/sys/class/leds/status" ] && status_led="status"
	[ -d "/sys/class/leds/power:green" ] && status_led="power:green"
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

		[ "$status_led" = "power:green" ] && {
			led_off "power:red"
		}
		;;
	esac
}
