#!/bin/sh

commit=0

if [ -z "$(uci -q get uhttpd.main.ubus_prefix)" ]; then
	uci set uhttpd.main.ubus_prefix=/ubus
	commit=1
fi

[ "$(uci -q get uhttpd.main.ubus_socket)" = "/var/run/ubus.sock" ] && {
	uci set uhttpd.main.ubus_socket='/var/run/ubus/ubus.sock'
	commit=1
}

[ "$commit" = 1 ] && uci commit uhttpd && /etc/init.d/uhttpd reload

exit 0
