#!/bin/sh

[ "$DSL_NOTIFICATION_TYPE" = "DSL_INTERFACE_STATUS" ] || exit 0

. /lib/functions.sh
. /lib/functions/leds.sh

led_dsl_up() {
	case "$(config_get led_dsl trigger)" in
	"netdev")
		led_set_attr $1 "trigger" "netdev"
		led_set_attr $1 "device_name" "$(config_get led_dsl dev)"
		for m in $(config_get led_dsl mode); do
			led_set_attr $1 "$m" "1"
		done
		;;
	*)
		led_on $1
		;;
	esac
}

config_load system
config_get led led_dsl sysfs
if [ -n "$led" ]; then
	case "$DSL_INTERFACE_STATUS" in
	  "HANDSHAKE")  led_timer $led 500 500;;
	  "TRAINING")   led_timer $led 200 200;;
	  "UP")		led_dsl_up $led;;
	  *)		led_off $led
	esac
fi
