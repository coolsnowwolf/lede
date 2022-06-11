#!/bin/sh

[ "$(uci get rpcd.@rpcd[0].socket)" = "/var/run/ubus.sock" ] || exit 0

uci set rpcd.@rpcd[0].socket='/var/run/ubus/ubus.sock'
uci commit rpcd

exit 0
