#!/bin/sh /etc/rc.common
# NVRAM setup
#
# This file handles the NVRAM quirks of various hardware of the bcm53xx target.

START=02

clear_partialboots() {
	# clear partialboots

	case $(board_name) in
		linksys,panamera)
			COMMIT=1
			nvram set partialboots=0
			;;
	esac
}

set_wireless_led_behaviour() {
	# set Broadcom wireless LED behaviour for both radios
	# 0:ledbh9 -> Behaviour of 2.4GHz LED
	# 1:ledbh9 -> Behaviour of 5GHz LED
	# 0x7 makes the wireless LEDs on, when radios are enabled, and blink when there's activity

	case $(board_name) in
		asus,rt-ac88u)
			COMMIT=1
			nvram set 0:ledbh9=0x7 set 1:ledbh9=0x7
			;;
	esac
}

boot() {
	. /lib/functions.sh

	clear_partialboots
	set_wireless_led_behaviour

	[ "$COMMIT" = "1" ] && nvram commit
}
