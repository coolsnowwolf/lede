#!/bin/sh

. /lib/functions/system.sh
. /lib/functions/uci-defaults.sh

board_config_update

case "$(board_name)" in
	traverse,ten64)
		ucidef_set_interface_lan "eth0 eth1 eth2 eth3"
		ucidef_set_interface_wan "eth6"
		;;
esac

board_config_flush

exit 0
