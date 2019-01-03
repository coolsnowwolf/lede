#!/bin/sh
# Copyright (C) 2009-2013 OpenWrt.org

. /lib/functions.sh
. /lib/functions/leds.sh

get_status_led() {
	case $(board_name) in
	"akitio,mycloud")
		status_led="akitio:red:status"
		;;
	"mitrastar,stg-212")
		status_led="zyxel:blue:status"
		;;
	"shuttle,kd20")
		status_led="kd20:blue:status"
		;;
	"cloudengines,pogoplug"*)
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
