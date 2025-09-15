#!/bin/sh
INTERFACE="$1"

[ -n "$INTERFACE" ] || {
	echo "Usage: $0 <interface>"
	exit 1
}

ubus -S list "network.interface.$INTERFACE" >/dev/null || {
	echo "Interface $INTERFACE not found"
	exit 1
}
ubus call network.interface status "{ \"interface\" : \"$INTERFACE\" }"
