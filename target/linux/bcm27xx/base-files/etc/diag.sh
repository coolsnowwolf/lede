#!/bin/sh
# Copyright (C) 2015-2016 OpenWrt.org
# Copyright (C) 2017 LEDE project

. /lib/functions.sh
. /lib/functions/leds.sh

status_led_restore_trigger() {
	local led_lc=$(echo "$status_led" | awk '{print tolower($0)}')
	local led_path="/proc/device-tree/leds/led-$led_lc"
	local led_trigger

	[ -d "$led_path" ] && \
		led_trigger=$(cat "$led_path/linux,default-trigger" 2>/dev/null)

	[ -n "$led_trigger" ] && \
		led_set_attr $status_led "trigger" "$led_trigger"
}

set_state() {
	if [ -d "/sys/class/leds/ACT" ]; then
		status_led="ACT"
	else
		return
	fi

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
	upgrade)
		status_led_blink_preinit_regular
		;;
	done)
		status_led_restore_trigger
		;;
	esac
}
