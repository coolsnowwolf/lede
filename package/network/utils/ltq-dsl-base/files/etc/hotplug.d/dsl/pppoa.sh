#!/bin/sh

[ "$DSL_NOTIFICATION_TYPE" = "DSL_INTERFACE_STATUS" ] || exit 0

. /usr/share/libubox/jshn.sh
. /lib/functions.sh

include /lib/network
scan_interfaces

interfaces=$(ubus list network.interface.\* | cut -d"." -f3)
for ifc in $interfaces; do

	json_load "$(ifstatus $ifc)"

	json_get_var proto proto
	if [ "$proto" != "pppoa" ]; then
		continue
	fi

	json_get_var up up
	config_get_bool auto "$ifc" auto 1
	if [ "$DSL_INTERFACE_STATUS" = "UP" ]; then
		if [ "$up" != 1 ] && [ "$auto" = 1 ]; then
			( sleep 1; ifup "$ifc" ) &
		fi
	else
		if [ "$up" = 1 ] && [ "$auto" = 1 ]; then
			( sleep 1; ifdown "$ifc" ) &
		else
			json_get_var autostart autostart
			if [ "$up" != 1 ] && [ "$autostart" = 1 ]; then
				( sleep 1; ifdown "$ifc" ) &
			fi
		fi
	fi
done
