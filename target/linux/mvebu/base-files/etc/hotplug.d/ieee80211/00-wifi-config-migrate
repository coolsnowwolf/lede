#!/bin/sh

# The pcie-controller device was renamed to pcie in Linux kernel 4.14
# commit 28fbb9c539e2 ("ARM: dts: marvell: fix PCI bus dtc warnings").
# This script migrates the path in the UCI configuration from the old
# name to the new name and also back, when am upgrade or downgrade is
# done. It checks if the name exists before changing the configuration.
# This has to be done before the 10-wifi-detect script from mac80211 is
# executed because this would add the devices again under the new path
# name.

. /lib/functions.sh

PATH_CHANGED=0

rename_wifi_path() {
	local path_old=$(uci get wireless.${1}.path)
	local path_new=$(echo ${path_old} | sed "${2}")

	if [ -e "/sys/devices/platform/${path_new}" ] && [ ${path_old} != ${path_new} ]
	then 
		uci set wireless.${1}.path=${path_new}
		PATH_CHANGED=1
	fi
}

rename_wifi_path_list() {
	# migration from kernel 4.9 to 4.14
	rename_wifi_path $1 "s/soc:pcie-controller/soc:pcie/"
	# migration from kernel 4.14 to 4.9
	rename_wifi_path $1 "s/soc:pcie/soc:pcie-controller/"
}

[ "${ACTION}" = "add" ] && {
	[ ! -e /etc/config/wireless ] && return

	config_load wireless
	config_foreach rename_wifi_path_list wifi-device

	[ "$PATH_CHANGED" = "1" ] && uci commit wireless
}
