#!/bin/sh

[ -e /lib/firmware/$FIRMWARE ] && exit 0

. /lib/functions/caldata.sh

board=$(board_name)

case "$FIRMWARE" in
"ath9k-eeprom-pci-0000:43:00.0.bin")
        case $board in
        netgear,wndr4700)
		. /lib/upgrade/nand.sh

		if [ -n "$(nand_find_volume ubi0 caldata)" ]; then
			caldata_extract_ubi "caldata" 0x1000 0x1000
		else
			caldata_extract "wifi_data" 0x1000 0x1000
			ath9k_patch_mac $(mtd_get_mac_binary wifi_data 0x0)
		fi
		;;
	*)
		caldata_die "board $board is not supported yet"
		;;
	esac
	;;

"ath9k-eeprom-pci-0000:44:00.0.bin")
        case $board in
        netgear,wndr4700)
		. /lib/upgrade/nand.sh

		if [ -n "$(nand_find_volume ubi0 caldata)" ]; then
			caldata_extract_ubi "caldata" 0x5000 0x1000
		else
			caldata_extract "wifi_data" 0x5000 0x1000
			ath9k_patch_mac $(mtd_get_mac_binary wifi_data 0xc)
		fi
		;;
	*)
		caldata_die "board $board is not supported yet"
		;;
	esac
	;;
esac
