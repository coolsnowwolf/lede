#!/bin/sh
#
# Copyright (C) 2013-2015 OpenWrt.org
#

. /lib/functions/uci-defaults.sh
. /lib/functions/system.sh

amlogic_setup_interfaces()
{
	local board="$1"

	case "$board" in
	*)
		ucidef_set_interface_lan 'eth0'
		;;
	esac
}

generate_mac_from_mmc()
{
	local sd_hash
	local bootdisk=$(
		. /lib/upgrade/common.sh
		export_bootdevice && export_partdevice bootdisk 0 && echo $bootdisk
	)
	if echo "$bootdisk" | grep -q '^mmcblk' && [ -f "/sys/class/block/$bootdisk/device/cid" ]; then
		sd_hash=$(sha256sum /sys/class/block/$bootdisk/device/cid | head -n 1)
	else
		sd_hash=$(sha256sum /sys/class/block/mmcblk*/device/cid | head -n 1)
	fi
	local mac_base=$(macaddr_canonicalize "$(echo "${sd_hash}" | dd bs=1 count=12 2>/dev/null)")
	echo "$(macaddr_unsetbit_mc "$(macaddr_setbit_la "${mac_base}")")"
}

amlogic_setup_macs()
{
	local board="$1"
	local lan_mac=""
	local wan_mac=""

	case "$board" in
	*)
		lan_mac=$(generate_mac_from_mmc)
		;;
	esac

	[ -n "$wan_mac" ] && ucidef_set_interface_macaddr "wan" $wan_mac
	[ -n "$lan_mac" ] && ucidef_set_interface_macaddr "lan" $lan_mac
}


board_config_update
board=$(board_name)
amlogic_setup_interfaces $board
amlogic_setup_macs $board
board_config_flush

exit 0
