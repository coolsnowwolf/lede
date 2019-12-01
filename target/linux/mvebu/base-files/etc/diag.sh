#!/bin/sh
# Copyright (C) 2014-2016 OpenWrt.org
# Copyright (C) 2016 LEDE-Project.org

. /lib/functions.sh
. /lib/functions/leds.sh

get_status_led() {
	case $(board_name) in
	linksys,caiman)
		status_led="caiman:white:power"
		;;
	linksys,cobra)
		status_led="cobra:white:power"
		;;
	linksys,mamba)
		status_led="mamba:white:power"
		;;
	linksys,rango)
		status_led="rango:white:power"
		;;
	linksys,shelby)
		status_led="shelby:white:power"
		;;
	linksys,venom)
		status_led="venom:blue:power"
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
