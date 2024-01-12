#!/bin/sh
# Copyright (C) 2015-2016 OpenWrt.org
# Copyright (C) 2017 LEDE project

. /lib/functions.sh
. /lib/functions/leds.sh

set_state() {
	case "$(board_name)" in
	raspberrypi,2-model-b |\
	raspberrypi,2-model-b-rev2 |\
	raspberrypi,3-model-b |\
	raspberrypi,3-model-b-plus |\
	raspberrypi,400 |\
	raspberrypi,4-compute-module |\
	raspberrypi,4-model-b |\
	raspberrypi,model-b-plus)
		status_led="led1"
		;;
	raspberrypi,3-compute-module |\
	raspberrypi,model-b |\
	raspberrypi,model-zero |\
	raspberrypi,model-zero-2 |\
	raspberrypi,model-zero-w)
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
