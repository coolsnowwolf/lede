#!/bin/sh

[ -e /lib/firmware/$FIRMWARE ] && exit 0

. /lib/functions/caldata.sh

board=$(board_name)

case "$FIRMWARE" in
"ath11k/IPQ6018/hw1.0/cal-ahb-c000000.wifi.bin")
	case "$board" in
	glinet,gl-ax1800|\
	glinet,gl-axt1800|\
	linksys,mr7350|\
	qihoo,360v6|\
	xiaomi,rm1800)
		caldata_extract "0:art" 0x1000 0x10000
		;;
	jdcloud,re-cs-02|\
	jdcloud,re-ss-01|\
	redmi,ax5-jdcloud)
		caldata_extract_mmc "0:ART" 0x1000 0x10000
		;;
	esac
	;;
"ath11k/QCN9074/hw1.0/cal-pci-0000:01:00.0.bin")
	case "$board" in
	jdcloud,re-cs-02)
		caldata_extract_mmc "0:ART" 0x26800 0x20000
		;;
	esac
	;;
*)
	exit 1
	;;
esac
