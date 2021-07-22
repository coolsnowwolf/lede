#!/bin/sh

[ -e /lib/firmware/$FIRMWARE ] && exit 0

. /lib/functions/caldata.sh

board=$(board_name)

case "$FIRMWARE" in
"ath9k-eeprom-pci-0000:00:00.0.bin")
	case $board in
	buffalo,whr-g301n|\
	engenius,eap350-v1|\
	engenius,ecb350-v1|\
	engenius,enh202-v1|\
	tplink,tl-wa701nd-v1|\
	tplink,tl-wa730re-v1|\
	tplink,tl-wa801nd-v1|\
	tplink,tl-wa830re-v1|\
	tplink,tl-wa901nd-v1|\
	tplink,tl-wr841-v5|\
	tplink,tl-wr941-v4)
		caldata_extract "art" 0x1000 0xeb8
		;;
	dlink,dir-615-e4)
		caldata_extract "art" 0x1000 0x1000
		ath9k_patch_mac_crc $(mtd_get_mac_ascii "nvram" "lan_mac") 0x10c
		;;
	netgear,wnr1000-v2|\
	netgear,wnr2000-v3|\
	netgear,wnr612-v2|\
	on,n150r|\
	tplink,tl-mr3220-v1|\
	tplink,tl-mr3420-v1|\
	tplink,tl-wr740n-v1|\
	tplink,tl-wr740n-v3|\
	tplink,tl-wr741-v1|\
	tplink,tl-wr743nd-v1|\
	tplink,tl-wr841-v7)
		caldata_extract "art" 0x1000 0x1000
		;;
	pqi,air-pen)
		caldata_extract "art" 0x1000 0x7d2
		;;
	*)
		caldata_die "board $board is not supported yet"
		;;
	esac
	;;
esac
