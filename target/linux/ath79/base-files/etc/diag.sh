#!/bin/sh

. /lib/functions/leds.sh

get_status_led() {
	local board=$(board_name)

	case $board in
	"glinet,ar150")
		status_led="gl-ar150:orange:wlan"
		;;
	"tplink,tl-wr1043nd-v1")
		status_led="tp-link:green:system"
		;;
	"ubnt,unifi")
		status_led="ubnt:green:dome"
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
