#!/bin/sh
# Copyright (C) 2006-2019 OpenWrt.org

. /lib/functions/leds.sh

boot="$(get_dt_led boot)"
failsafe="$(get_dt_led failsafe)"
running="$(get_dt_led running)"
upgrade="$(get_dt_led upgrade)"

set_led_state() {
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
			status_led="$running"
			status_led_off
		}
		status_led="$upgrade"
		status_led_blink_preinit_regular
		;;
	done)
		status_led_off
		[ "$status_led" != "$running" ] && \
			status_led_restore_trigger "boot"
		[ -n "$running" ] && {
			status_led="$running"
			status_led_on
		}
		;;
	esac
}

set_state() {
	[ -n "$boot" -o -n "$failsafe" -o -n "$running" -o -n "$upgrade" ] && set_led_state "$1"
}
