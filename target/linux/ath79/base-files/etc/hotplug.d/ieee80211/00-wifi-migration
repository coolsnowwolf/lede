#!/bin/sh

WIFI_PATH_CHANGED=0

. /lib/functions.sh

migrate_wifi_path() {
	local section="$1"
	local path

	config_get path ${section} path
	case ${path} in
		"pci0000:01/0000:01:00.0")
			board=$(board_name)

			case "$board" in
				tplink,archer-c5-v1|\
				tplink,archer-c7-v1|\
				tplink,archer-c7-v2|\
				zyxel,emg2926-q10a|\
				zyxel,nbg6716)
					path="pci0000:00/0000:00:00.0"
					WIFI_PATH_CHANGED=1
				;;
				*)
					return 0
				;;
			esac
		;;
		"platform/ahb/ahb:apb/18100000.wmac"|\
		"platform/ar933x_wmac"|\
		"platform/ar934x_wmac"|\
		"platform/qca953x_wmac"|\
		"platform/qca955x_wmac"|\
		"platform/qca956x_wmac")
			path="platform/ahb/18100000.wmac"
			WIFI_PATH_CHANGED=1
		;;
		"platform/ath9k")
			path="platform/ahb/180c0000.wmac"
			WIFI_PATH_CHANGED=1
		;;
		*)
			return 0
		;;
	esac

	uci set wireless.${section}.path=${path}
}

[ "${ACTION}" = "add" ] && {
	[ ! -e /etc/config/wireless ] && return 0

	config_load wireless
	config_foreach migrate_wifi_path wifi-device

	[ "${WIFI_PATH_CHANGED}" = "1" ] && uci commit wireless
}
