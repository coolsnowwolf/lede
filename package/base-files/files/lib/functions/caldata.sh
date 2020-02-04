# Copyright (C) 2019 OpenWrt.org

. /lib/functions.sh
. /lib/functions/system.sh

caldata_die() {
	echo "caldata: " "$*"
	exit 1
}

caldata_extract() {
	local part=$1
	local offset=$(($2))
	local count=$(($3))
	local mtd

	mtd=$(find_mtd_chardev $part)
	[ -n "$mtd" ] || caldata_die "no mtd device found for partition $part"

	dd if=$mtd of=/lib/firmware/$FIRMWARE iflag=skip_bytes bs=$count skip=$offset count=1 2>/dev/null || \
		caldata_die "failed to extract calibration data from $mtd"
}

caldata_extract_ubi() {
	local part=$1
	local offset=$(($2))
	local count=$(($3))
	local ubidev
	local ubi

	. /lib/upgrade/nand.sh

	ubidev=$(nand_find_ubi $CI_UBIPART)
	ubi=$(nand_find_volume $ubidev $part)
	[ -n "$ubi" ] || caldata_die "no UBI volume found for $part"

	dd if=/dev/$ubi of=/lib/firmware/$FIRMWARE iflag=skip_bytes bs=$count skip=$offset count=1 2>/dev/null || \
		caldata_die "failed to extract calibration data from $ubi"
}

caldata_extract_reverse() {
	local part=$1
	local offset=$2
	local count=$(($3))
	local mtd
	local reversed
	local caldata

	mtd=$(find_mtd_chardev "$part")
	reversed=$(hexdump -v -s $offset -n $count -e '/1 "%02x "' $mtd)

	for byte in $reversed; do
		caldata="\x${byte}${caldata}"
	done

	printf "%b" "$caldata" > /lib/firmware/$FIRMWARE
}

caldata_from_file() {
	local source=$1
	local offset=$(($2))
	local count=$(($3))

	dd if=$source of=/lib/firmware/$FIRMWARE iflag=skip_bytes bs=$count skip=$offset count=1 2>/dev/null || \
		caldata_die "failed to extract calibration data from $source"
}

caldata_valid() {
	local expected="$1"

	magic=$(hexdump -v -n 2 -e '1/1 "%02x"' /lib/firmware/$FIRMWARE)
	[ "$magic" = "$expected" ]
	return $?
}

caldata_patch_chksum() {
	local mac=$1
	local mac_offset=$(($2))
	local chksum_offset=$(($3))
	local xor_mac
	local xor_fw_mac
	local xor_fw_chksum

	xor_mac=${mac//:/}
	xor_mac="${xor_mac:0:4} ${xor_mac:4:4} ${xor_mac:8:4}"

	xor_fw_mac=$(hexdump -v -n 6 -s $mac_offset -e '/1 "%02x"' /lib/firmware/$FIRMWARE)
	xor_fw_mac="${xor_fw_mac:0:4} ${xor_fw_mac:4:4} ${xor_fw_mac:8:4}"

	xor_fw_chksum=$(hexdump -v -n 2 -s $chksum_offset -e '/1 "%02x"' /lib/firmware/$FIRMWARE)
	xor_fw_chksum=$(xor $xor_fw_chksum $xor_fw_mac $xor_mac)

	printf "%b" "\x${xor_fw_chksum:0:2}\x${xor_fw_chksum:2:2}" | \
		dd of=/lib/firmware/$FIRMWARE conv=notrunc bs=1 seek=$chksum_offset count=2
}

caldata_patch_mac() {
	local mac=$1
	local mac_offset=$(($2))
	local chksum_offset=$3

	[ -z "$mac" -o -z "$mac_offset" ] && return

	[ -n "$chksum_offset" ] && caldata_patch_chksum "$mac" "$mac_offset" "$chksum_offset"

	macaddr_2bin $mac | dd of=/lib/firmware/$FIRMWARE conv=notrunc oflag=seek_bytes bs=6 seek=$mac_offset count=1 || \
		caldata_die "failed to write MAC address to eeprom file"
}

ath9k_patch_mac() {
	local mac=$1

	caldata_patch_mac "$mac" 0x2
}

ath9k_patch_mac_crc() {
	local mac=$1
	local mac_offset=$2
	local chksum_offset=$((mac_offset - 10))

	caldata_patch_mac "$mac" "$mac_offset" "$chksum_offset"
}

ath10k_patch_mac() {
	local mac=$1

	caldata_patch_mac "$mac" 0x6 0x2
}
