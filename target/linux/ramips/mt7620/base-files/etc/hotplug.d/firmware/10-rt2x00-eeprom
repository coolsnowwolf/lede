#!/bin/sh

[ -e /lib/firmware/$FIRMWARE ] && exit 0

. /lib/functions/caldata.sh

jboot_eeprom_extract() {
	local part=$1
	local offset=$2
	local mtd

	mtd=$(find_mtd_part $part)
	[ -n "$mtd" ] || \
		caldata_die "no mtd device found for partition $part"

	jboot_config_read -i $mtd -o $offset -e /lib/firmware/$FIRMWARE  2>/dev/null || \
		caldata_die "failed to extract from $mtd"
}

board=$(board_name)

case "$FIRMWARE" in
"soc_wmac.eeprom")
	case $board in
	dlink,dir-510l|\
	dlink,dwr-116-a1|\
	dlink,dwr-118-a1|\
	dlink,dwr-118-a2|\
	dlink,dwr-921-c1|\
	dlink,dwr-922-e2|\
	dlink,dwr-960|\
	lava,lr-25g001)
		wan_mac=$(jboot_config_read -m -i $(find_mtd_part "config") -o 0xE000)
		wifi_mac=$(macaddr_add "$wan_mac" 1)
		jboot_eeprom_extract "config" 0xE000
		caldata_patch_mac $wifi_mac 0x4
		;;
	dovado,tiny-ac)
		wifi_mac=$(mtd_get_mac_ascii u-boot-env INIC_MAC_ADDR)
		caldata_extract "factory" 0x0 0x200
		caldata_patch_mac $wifi_mac 0x4
		;;
	*)
		caldata_die "Please define mtd-eeprom in $board DTS file!"
		;;
	esac
	;;
esac
