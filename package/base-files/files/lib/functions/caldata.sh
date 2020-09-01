# Copyright (C) 2019 OpenWrt.org

. /lib/functions.sh
. /lib/functions/system.sh

caldata_dd() {
	local source=$1
	local target=$2
	local count=$(($3))
	local offset=$(($4))

	dd if=$source of=$target iflag=skip_bytes,fullblock bs=$count skip=$offset count=1 2>/dev/null
	return $?
}

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

	caldata_dd $mtd /lib/firmware/$FIRMWARE $count $offset || \
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

	caldata_dd /dev/$ubi /lib/firmware/$FIRMWARE $count $offset || \
		caldata_die "failed to extract calibration data from $ubi"
}

caldata_extract_mmc() {
	local part=$1
	local offset=$(($2))
	local count=$(($3))
	local mmc_part

	mmc_part=$(find_mmc_part $part)
	[ -n "$mmc_part" ] || caldata_die "no mmc partition found for partition $part"

	caldata_dd $mmc_part /lib/firmware/$FIRMWARE $count $offset || \
		caldata_die "failed to extract calibration data from $mmc_part"
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
	local target=$4

	[ -n "$target" ] || target=/lib/firmware/$FIRMWARE

	caldata_dd $source $target $count $offset || \
		caldata_die "failed to extract calibration data from $source"
}

caldata_sysfsload_from_file() {
	local source=$1
	local offset=$(($2))
	local count=$(($3))
	local target_dir="/sys/$DEVPATH"
	local target="$target_dir/data"

	[ -d "$target_dir" ] || \
		caldata_die "no sysfs dir to write: $target"

	echo 1 > "$target_dir/loading"
	caldata_dd $source $target $count $offset
	if [ $? != 0 ]; then
		echo 1 > "$target_dir/loading"
		caldata_die "failed to extract calibration data from $source"
	else
		echo 0 > "$target_dir/loading"
	fi
}

caldata_valid() {
	local expected="$1"
	local target=$2

	[ -n "$target" ] || target=/lib/firmware/$FIRMWARE

	magic=$(hexdump -v -n 2 -e '1/1 "%02x"' $target)
	[ "$magic" = "$expected" ]
	return $?
}

caldata_patch_chksum() {
	local mac=$1
	local mac_offset=$(($2))
	local chksum_offset=$(($3))
	local target=$4
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
		dd of=$target conv=notrunc bs=1 seek=$chksum_offset count=2
}

caldata_patch_mac() {
	local mac=$1
	local mac_offset=$(($2))
	local chksum_offset=$3
	local target=$4

	[ -z "$mac" -o -z "$mac_offset" ] && return

	[ -n "$target" ] || target=/lib/firmware/$FIRMWARE

	[ -n "$chksum_offset" ] && caldata_patch_chksum "$mac" "$mac_offset" "$chksum_offset" "$target"

	macaddr_2bin $mac | dd of=$target conv=notrunc oflag=seek_bytes bs=6 seek=$mac_offset count=1 || \
		caldata_die "failed to write MAC address to eeprom file"
}

ath9k_patch_mac() {
	local mac=$1
	local target=$2

	caldata_patch_mac "$mac" 0x2 "" "$target"
}

ath9k_patch_mac_crc() {
	local mac=$1
	local mac_offset=$2
	local chksum_offset=$((mac_offset - 10))
	local target=$4

	caldata_patch_mac "$mac" "$mac_offset" "$chksum_offset" "$target"
}

ath10k_patch_mac() {
	local mac=$1
	local target=$2

	caldata_patch_mac "$mac" 0x6 0x2 "$target"
}
