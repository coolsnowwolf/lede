#!/bin/sh

SEC=$(uci -q get 3ginfo.@3ginfo[0].network)
[ -z "$SEC" ] && exit 0

ALLOW=$(uci -q get 3ginfo.@3ginfo[0].connect_button)
[ "x$ALLOW" = "x0" ] && exit 0

UP=$(ifstatus $SEC | grep "\"up\": true")
if [ -n "$UP" ]; then
	ifdown $SEC > /dev/null 2>&1
else
	ifup $SEC > /dev/null 2>&1
fi

sleep 3
exit 0
