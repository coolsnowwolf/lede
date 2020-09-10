#!/bin/sh

[ -e /lib/firmware/$FIRMWARE ] && exit 0

. /lib/functions/caldata.sh

wlan_data="/sys/firmware/mikrotik/hard_config/wlan_data"

board=$(board_name)

case "$FIRMWARE" in
"ath10k/cal-pci-0000:00:00.0.bin")
	case $board in
	mikrotik,routerboard-921gs-5hpacd-15s|\
	mikrotik,routerboard-wap-g-5hact2hnd)
		caldata_sysfsload_from_file $wlan_data 0x5000 0x844
		;;
	esac
	;;
"ath10k/cal-pci-0000:01:00.0.bin")
	case $board in
	mikrotik,routerboard-922uags-5hpacd)
		caldata_sysfsload_from_file $wlan_data 0x5000 0x844
		;;
	esac
	;;
*)
	exit 1
	;;
esac
