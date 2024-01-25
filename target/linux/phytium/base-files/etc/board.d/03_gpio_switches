#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later

. /lib/functions/uci-defaults.sh

board_config_update

board=$(board_name)

case "$board" in
traverse,ten64)
	ucidef_add_gpio_switch "lte_reset" "Cell Modem Reset" "376"
	ucidef_add_gpio_switch "lte_power" "Cell Modem Power" "377"
	ucidef_add_gpio_switch "lte_disable" "Cell Modem Airplane mode" "378"
	ucidef_add_gpio_switch "gnss_disable" "Cell Modem Disable GNSS receiver" "379"
	ucidef_add_gpio_switch "lower_sfp_txidsable" "Lower SFP+ TX Disable" "369"
	ucidef_add_gpio_switch "upper_sfp_txdisable" "Upper SFP+ TX Disable" "373"
	;;
esac

board_config_flush

exit 0
