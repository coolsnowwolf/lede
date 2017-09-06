#!/bin/sh

[ "${ACTION}" = "released" -o -n "${TYPE}" ] || exit 0

. /lib/functions.sh

rfkill_state=0

wifi_rfkill_set() {
	uci set wireless.$1.disabled=$rfkill_state
}

wifi_rfkill_check() {
	local disabled
	config_get disabled $1 disabled
	[ "$disabled" = "1" ] || rfkill_state=1
}

config_load wireless
case "${TYPE}" in
"switch")
	[ "${ACTION}" = "released" ] && rfkill_state=1
	;;
*)
	config_foreach wifi_rfkill_check wifi-device
	;;
esac
config_foreach wifi_rfkill_set wifi-device
uci commit wireless
wifi up

return 0
