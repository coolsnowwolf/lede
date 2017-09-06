#!/bin/sh
. /usr/share/libubox/jshn.sh
DEVICE="$1"

[ -n "$DEVICE" ] || {
	echo "Usage: $0 <device>"
	exit 1
}

json_init
json_add_string name "$DEVICE"
ubus call network.device status "$(json_dump)"
