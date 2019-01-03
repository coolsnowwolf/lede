#!/bin/sh

wps_catch_credentials() {
	local iface ifaces ifc ifname ssid encryption key radio radios
	local found=0

	. /usr/share/libubox/jshn.sh
	ubus -S -t 30 listen wps_credentials | while read creds; do
		json_init
		json_load "$creds"
		json_select wps_credentials || continue
		json_get_vars ifname ssid key encryption
		local ifcname="$ifname"
		json_init
		json_load "$(ubus -S call network.wireless status)"
		json_get_keys radios
		for radio in $radios; do
			json_select $radio
			json_select interfaces
			json_get_keys ifaces
			for ifc in $ifaces; do
				json_select $ifc
				json_get_vars ifname
				[ "$ifname" = "$ifcname" ] && {
					ubus -S call uci set "{\"config\":\"wireless\", \"type\":\"wifi-iface\",		\
								\"match\": { \"device\": \"$radio\", \"encryption\": \"wps\" },	\
								\"values\": { \"encryption\": \"$encryption\", 			\
										\"ssid\": \"$ssid\", 				\
										\"key\": \"$key\" } }"
					ubus -S call uci commit '{"config": "wireless"}'
					ubus -S call uci apply
				}
				json_select ..
			done
			json_select ..
			json_select ..
		done
	done
}

if [ "$ACTION" = "pressed" -a "$BUTTON" = "wps" ]; then
	wps_done=0
	ubusobjs="$( ubus -S list hostapd.* )"
	for ubusobj in $ubusobjs; do
		ubus -S call $ubusobj wps_start && wps_done=1
	done
	[ $wps_done = 0 ] || return 0
	wps_done=0
	ubusobjs="$( ubus -S list wpa_supplicant.* )"
	for ubusobj in $ubusobjs; do
		ubus -S call $ubusobj wps_start && wps_done=1
	done
	[ $wps_done = 0 ] || wps_catch_credentials &
fi

return 0
