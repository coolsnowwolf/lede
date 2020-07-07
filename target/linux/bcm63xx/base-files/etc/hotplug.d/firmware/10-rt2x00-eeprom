#!/bin/sh
# Based on gabors ralink wisoc implementation.

[ -e /lib/firmware/$FIRMWARE ] && exit 0

. /lib/functions/caldata.sh

board=$(board_name)

case "$FIRMWARE" in
"rt2x00.eeprom" )
	case $board in
	huawei,echolife-hg556a-c)
		caldata_extract "cal_data" 0x1fe00 0x200
		;;
	huawei,echolife-hg622|\
	huawei,echolife-hg655b)
		caldata_extract "cal_data" 0x0 0x200
		;;
	*)
		caldata_die "board $board is not supported yet"
		;;
	esac
	;;
esac
