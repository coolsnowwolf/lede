#!/bin/sh
# Copyright (C) 2015-2016 OpenWrt.org
# Copyright (C) 2017 LEDE project

. /lib/functions.sh
. /lib/functions/leds.sh

set_state() {
	case "$(board_name)" in
	rpi-2-b |\
	rpi-b-plus)
		status_led="led1"
		;;
	rpi-b |\
	rpi-cm |\
	rpi-zero |\
	rpi-zero-w)
		status_led="led0"
		;;
	esac

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
