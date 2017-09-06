#!/bin/sh
# Copyright (C) 2014 OpenWrt.org

. /lib/functions.sh
. /lib/functions/leds.sh

get_status_led() {
	case $(board_name) in
	dockstar|\
	goflexhome|\
	goflexnet)
		status_led="status:orange:fault"
		;;
	pogo_e02)
		status_led="pogo_e02:orange:fault"
		;;
	linksys-audi)
		status_led="audi:green:power"
		;;
	linksys-viper)
		status_led="viper:white:health"
		;;
	nsa310b)
		status_led="nsa310:green:sys"
		;;
	on100)
		status_led="on100:green:health"
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
