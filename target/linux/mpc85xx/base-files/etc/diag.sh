#!/bin/sh
# Copyright (C) 2013 OpenWrt.org

. /lib/functions.sh
. /lib/functions/leds.sh

get_status_led() {
	case $(board_name) in
	aerohive,hiveap-330)
		status_led="hiveap-330:green:tricolor0"
		;;
	tplink,tl-wdr4900-v1)
		status_led="tp-link:blue:system"
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
