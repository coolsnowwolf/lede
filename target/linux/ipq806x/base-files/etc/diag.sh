#!/bin/sh
# Copyright (C) 2016 Henryk Heisig hyniu@o2.pl

. /lib/functions/leds.sh
. /lib/ipq806x.sh

boot="$(ipq806x_get_dt_led boot)"
failsafe="$(ipq806x_get_dt_led failsafe)"
running="$(ipq806x_get_dt_led running)"
upgrade="$(ipq806x_get_dt_led upgrade)"

set_state() {
	status_led="$boot"

	case "$1" in
	preinit)
		status_led_blink_preinit
		;;
	failsafe)
		status_led_off
		[ -n "$running" ] && {
			status_led="$running"
			status_led_off
		}
		status_led="$failsafe"
		status_led_blink_failsafe
		;;
	preinit_regular)
		status_led_blink_preinit_regular
		;;
	upgrade)
		[ -n "$running" ] && {
			status_led="$upgrade"
			status_led_blink_preinit_regular
		}
		;;
	done)
		status_led_off
		[ -n "$running" ] && {
			status_led="$running"
			status_led_on
		}
		;;
	esac
}
