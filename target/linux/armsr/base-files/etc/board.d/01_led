#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later

. /lib/functions/uci-defaults.sh

board_config_update

board=$(board_name)

case "$board" in
traverse,ten64)
	ucidef_set_led_netdev "sfp1" "SFP 1" "ten64:green:sfp1:down" "eth8" "link tx rx"
	ucidef_set_led_netdev "sfp2" "SFP 2" "ten64:green:sfp2:up" "eth9" "link tx rx"
	;;
esac

board_config_flush

exit 0
