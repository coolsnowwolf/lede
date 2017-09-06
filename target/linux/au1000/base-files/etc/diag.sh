#!/bin/sh
# Copyright (C) 2007-2013 OpenWrt.org

. /lib/functions/leds.sh

set_state() {
	case "$1" in
	preinit)
		led_off "mtx1:green"
		led_on "mtx1:red"
		;;
	failsafe)
		led_on "mtx1:green"
		led_on "mtx1:red"
		;;
	done)
		led_on "mtx1:green"
		led_off "mtx1:red"
		;;
	esac
}
