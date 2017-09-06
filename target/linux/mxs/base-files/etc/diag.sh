#!/bin/sh
# Copyright (C) 2013-2014 OpenWrt.org

. /lib/functions.sh
. /lib/functions/leds.sh

get_status_led() {
	case $(board_name) in
	duckbill)
		status_led="duckbill:green:status"
		;;
	olinuxino)
		status_led="green"
		;;
	*)
		status_led=$(cd /sys/class/leds && ls -1d *:status 2> /dev/null | head -n 1)
		;;
	esac
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
