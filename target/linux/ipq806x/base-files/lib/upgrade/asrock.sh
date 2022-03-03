. /lib/functions.sh

asrock_bootconfig_mangle() {
	local mtdnum="$(find_mtd_index 0:bootconfig)"
	# XXX: drop upper case after kernel v5.4 is gone (qcom-smem)
	[ -n "$mtdnum" ] || mtdnum="$(find_mtd_index 0:BOOTCONFIG)"

	if [ -z "$mtdnum" ]; then
		echo "cannot find bootconfig mtd partition"
		return 1
	fi
	dd if=/dev/mtd$mtdnum of=/tmp/mtd$mtdnum bs=1k

	local partition_byte="$(dd if=/tmp/mtd$mtdnum bs=1 skip=52 count=1)"
	local upgrade_byte="$(dd if=/tmp/mtd$mtdnum bs=1 skip=4 count=1)"

	if [ $1 = "bootcheck" ]; then
		if [ ! -s $upgrade_byte ]; then
			dd if=/dev/mtd$mtdnum of=/tmp/mtd$mtdnum bs=1k
			printf '\x00' | dd of=/tmp/mtd$mtdnum conv=notrunc bs=1 seek=4
			printf '\x00' | dd of=/tmp/mtd$mtdnum conv=notrunc bs=1 seek=56
		else
			return 1
		fi
	elif [ $1 = "sysupgrade" ]; then
		printf '\x01' | dd of=/tmp/mtd$mtdnum conv=notrunc bs=1 seek=4
		printf '\x01' | dd of=/tmp/mtd$mtdnum conv=notrunc bs=1 seek=56
	fi

	if [ -s $partition_byte ]; then
		printf '\x01' | dd of=/tmp/mtd$mtdnum conv=notrunc bs=1 seek=52
	else
		printf '\x00' | dd of=/tmp/mtd$mtdnum conv=notrunc bs=1 seek=52
	fi

	mtd write /tmp/mtd$mtdnum /dev/mtd$mtdnum
	return 0
}

asrock_upgrade_prepare() {
	local ubidev="$( nand_find_ubi ubi )"

	#Set upgrade flag. If something goes wrong, router will boot with
	#factory firmware.
	asrock_bootconfig_mangle 'sysupgrade'

	if [ $? -ne 0 ]; then
		echo "cannot find bootconfig mtd partition"
		exit 1
	fi

	# Just delete these partitions if present and use
	# OpenWrt's standard names for those.
	ubirmvol /dev/$ubidev -N ubi_rootfs &> /dev/null || true
	ubirmvol /dev/$ubidev -N ubi_rootfs_data &> /dev/null || true
}
