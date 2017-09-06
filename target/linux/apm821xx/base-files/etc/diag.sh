#!/bin/sh

. /lib/functions.sh
. /lib/functions/leds.sh

get_status_led() {
	local board=$(board_name)

	case $board in
	mbl|\
	mr24|\
	mx60|\
	wndr4700)
		status_led="$board:green:power"
		;;

	*)
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
