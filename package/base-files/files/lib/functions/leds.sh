# Copyright (C) 2013 OpenWrt.org

get_dt_led_path() {
	local ledpath
	local basepath="/proc/device-tree"
	local nodepath="$basepath/aliases/led-$1"

	[ -f "$nodepath" ] && ledpath=$(cat "$nodepath")
	[ -n "$ledpath" ] && ledpath="$basepath$ledpath"

	echo "$ledpath"
}

get_dt_led_color_func() {
	local enum
	local func
	local idx
	local label

	[ -e "$1/function" ] && func=$(cat "$1/function")
	[ -e "$1/color" ] && idx=$((0x$(hexdump -n 4 -e '4/1 "%02x"' "$1/color")))
	[ -e "$1/function-enumerator" ] && \
		enum=$((0x$(hexdump -n 4 -e '4/1 "%02x"' "$1/function-enumerator")))

	[ -z "$idx" ] && [ -z "$func" ] && return 2

	if [ -n "$idx" ]; then
		for color in "white" "red" "green" "blue" "amber" \
			     "violet" "yellow" "ir" "multicolor" "rgb" \
			     "purple" "orange" "pink" "cyan" "lime"
		do
			[ $idx -eq 0 ] && label="$color" && break
			idx=$((idx-1))
		done
	fi

	label="$label:$func"
	[ -n "$enum" ] && label="$label-$enum"
	echo "$label"

	return 0
}

get_dt_led() {
	local label
	local ledpath=$(get_dt_led_path $1)

	[ -n "$ledpath" ] && \
		label=$(cat "$ledpath/label" 2>/dev/null) || \
		label=$(cat "$ledpath/chan-name" 2>/dev/null) || \
		label=$(get_dt_led_color_func "$ledpath") || \
		label=$(basename "$ledpath")

	echo "$label"
}

led_set_attr() {
	[ -f "/sys/class/leds/$1/$2" ] && echo "$3" > "/sys/class/leds/$1/$2"
}

led_timer() {
	led_set_attr $1 "trigger" "timer"
	led_set_attr $1 "delay_on" "$2"
	led_set_attr $1 "delay_off" "$3"
}

led_on() {
	led_set_attr $1 "trigger" "none"
	led_set_attr $1 "brightness" 255
}

led_off() {
	led_set_attr $1 "trigger" "none"
	led_set_attr $1 "brightness" 0
}

status_led_restore_trigger() {
	local trigger
	local ledpath=$(get_dt_led_path $1)

	[ -n "$ledpath" ] && \
		trigger=$(cat "$ledpath/linux,default-trigger" 2>/dev/null)

	[ -n "$trigger" ] && \
		led_set_attr "$(get_dt_led $1)" "trigger" "$trigger"
}

status_led_set_timer() {
	led_timer $status_led "$1" "$2"
	[ -n "$status_led2" ] && led_timer $status_led2 "$1" "$2"
}

status_led_set_heartbeat() {
	led_set_attr $status_led "trigger" "heartbeat"
}

status_led_on() {
	led_on $status_led
	[ -n "$status_led2" ] && led_on $status_led2
}

status_led_off() {
	led_off $status_led
	[ -n "$status_led2" ] && led_off $status_led2
}

status_led_blink_slow() {
	led_timer $status_led 1000 1000
}

status_led_blink_fast() {
	led_timer $status_led 100 100
}

status_led_blink_preinit() {
	led_timer $status_led 100 100
}

status_led_blink_failsafe() {
	led_timer $status_led 50 50
}

status_led_blink_preinit_regular() {
	led_timer $status_led 200 200
}
