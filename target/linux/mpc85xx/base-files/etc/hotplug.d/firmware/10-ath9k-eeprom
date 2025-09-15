#!/bin/sh

[ -e /lib/firmware/$FIRMWARE ] && exit 0

. /lib/functions/caldata.sh

board=$(board_name)

case "$FIRMWARE" in
"pci_wmac0.eeprom")
	case $board in
	tplink,tl-wdr4900-v1)
		caldata_extract "caldata" 0x1000 0x800
		ath9k_patch_mac $(mtd_get_mac_binary u-boot 0x4fc00)
		;;
	*)
		caldata_die "board $board is not supported yet"
		;;
	esac
	;;

"pci_wmac1.eeprom")
	case $board in
	tplink,tl-wdr4900-v1)
		caldata_extract "caldata" 0x5000 0x800
		ath9k_patch_mac $(macaddr_add $(mtd_get_mac_binary u-boot 0x4fc00) -1)
		;;
	*)
		caldata_die "board $board is not supported yet"
		;;
	esac
	;;
esac
