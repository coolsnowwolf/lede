#!/bin/sh

[ -e /lib/firmware/$FIRMWARE ] && exit 0

. /lib/functions/caldata.sh

board=$(board_name)

case "$FIRMWARE" in
"ath9k-eeprom-ahb-18100000.wmac.bin")
	case $board in
	8dev,rambutan)
		caldata_extract "caldata" 0x1000 0x800
		;;
	netgear,wndr3700-v4|\
	netgear,wndr4300|\
	netgear,wndr4300sw|\
	netgear,wndr4300tn|\
	netgear,wndr4300-v2|\
	netgear,wndr4500-v3)
		caldata_extract "caldata" 0x1000 0x440
		;;
	*)
		caldata_die "board $board is not supported yet"
		;;
	esac
	;;
"ath9k-eeprom-pci-0000:00:00.0.bin")
	case $board in
	netgear,wndr3700-v4|\
	netgear,wndr4300|\
	netgear,wndr4300sw|\
	netgear,wndr4300tn|\
	netgear,wndr4300-v2|\
	netgear,wndr4500-v3)
		caldata_extract "caldata" 0x5000 0x440
		;;
	*)
		caldata_die "board $board is not supported yet"
		;;
	esac
	;;
esac
