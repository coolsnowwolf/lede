#!/bin/sh
# Copyright (C) 2009-2013 OpenWrt.org

. /lib/functions.sh
. /lib/functions/leds.sh

get_status_led() {
	case $(board_name) in
	akitio)
		status_led="akitio:red:status"
		;;
	stg212)
		status_led="zyxel:blue:status"
		;;
	kd20)
		status_led="kd20:blue:status"
		;;
	pogoplug-pro | pogoplug-v3)
		status_led="pogoplug:blue:internal"
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
