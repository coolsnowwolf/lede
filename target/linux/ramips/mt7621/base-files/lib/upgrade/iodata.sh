#
# Copyright (C) 2019 OpenWrt.org
#

. /lib/functions.sh

iodata_mstc_prepare_fail() {
	echo "failed to check and prepare the environment, rebooting..."
	umount -a
	reboot -f
}

# I-O DATA devices manufactured by MSTC (MitraStar Technology Corp.)
# have two important flags:
# - bootnum: switch between two os images
#     use 1st image in OpenWrt
# - debugflag: enable/disable debug
#     users can interrupt Z-Loader for recovering the device if enabled
iodata_mstc_upgrade_prepare() {
	local persist_mtd="$(find_mtd_part persist)"
	local factory_mtd="$(find_mtd_part factory)"

	if [ -z "$persist_mtd" -o -z "$factory_mtd" ]; then
		echo 'cannot find mtd partition(s), "factory" or "persist"'
		iodata_mstc_prepare_fail
	fi

	local bootnum=$(hexdump -s 4 -n 1 -e '"%x"' ${persist_mtd})
	local debugflag=$(hexdump -s 65141 -n 1 -e '"%x"' ${factory_mtd})

	if [ "$bootnum" != "1" -a "$bootnum" != "2" ]; then
		echo "failed to get bootnum, please check the value at 0x4 in ${persist_mtd}"
		iodata_mstc_prepare_fail
	fi
	if [ "$debugflag" != "0" -a "$debugflag" != "1" ]; then
		echo "failed to get debugflag, please check the value at 0xFE75 in ${factory_mtd}"
		iodata_mstc_prepare_fail
	fi
	echo "current: bootnum => ${bootnum}, debugflag => ${debugflag}"

	if [ "$bootnum" = "2" ]; then
		if ! (echo -ne "\x01" | dd bs=1 count=1 seek=4 conv=notrunc of=${persist_mtd} 2>/dev/null); then
			echo "failed to set bootnum"
			iodata_mstc_prepare_fail
		fi
		echo "### switch to 1st os-image on next boot ###"
	fi
	if [ "$debugflag" = "0" ]; then
		if ! (echo -ne "\x01" | dd bs=1 count=1 seek=65141 conv=notrunc of=${factory_mtd} 2>/dev/null); then
			echo "failed to set debugflag"
			iodata_mstc_prepare_fail
		fi
		echo "### enable debug ###"
	fi
}
