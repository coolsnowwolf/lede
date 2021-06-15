#!/bin/sh

[ -e /lib/firmware/$FIRMWARE ] && exit 0

. /lib/functions/caldata.sh

caldata_extract_swap() {
	local part=$1
	local offset=$2
	local count=$3
	local mtd

	mtd=$(find_mtd_chardev $part)
	[ -n "$mtd" ] || caldata_die "no mtd device found for partition $part"

	offset=$(($offset / 2))
	count=$(($count / 2))

	dd if=$mtd of=/lib/firmware/$FIRMWARE bs=2 skip=$offset count=$count conv=swab 2>/dev/null || \
		caldata_die "failed to extract calibration data from $mtd"
}

case "$FIRMWARE" in
	"ath9k-eeprom-pci-0000:00:0e.0.bin" | \
	"ath9k-eeprom-pci-0000:01:00.0.bin" | \
	"ath9k-eeprom-pci-0000:02:00.0.bin")
		board=$(board_name)

		case "$board" in
			arcadyan,arv7518pw)
				caldata_extract_swap "boardconfig" 0x400 0x1000
				;;
			arcadyan,arv8539pw22)
				caldata_extract_swap "art" 0x400 0x1000
				;;
			bt,homehub-v2b)
				caldata_extract_swap "art" 0x0 0x1000
				ath9k_patch_mac_crc "00:00:00:00:00:00" 0x20c
				;;
			bt,homehub-v3a)
				caldata_extract_swap "art-copy" 0x0 0x1000
				ath9k_patch_mac_crc $(macaddr_add $(mtd_get_mac_ascii u-boot-env ethaddr) 2) 0x10c
				;;
			netgear,dgn3500|netgear,dgn3500b)
				caldata_extract "calibration" 0xf000 0x1000
				ath9k_patch_mac_crc $(macaddr_add $(mtd_get_mac_ascii u-boot-env ethaddr) 2) 0x20c
				;;
			avm,fritz7312|avm,fritz7320)
				caldata_extract "urlader" 0x985 0x1000
				;;
			*)
				caldata_die "board $board is not supported yet"
				;;
		esac
		;;
esac
