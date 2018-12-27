#!/bin/sh
# Copyright (C) 2018 OpenWrt.org
#

. /lib/functions.sh

# The mtd partition 'ubi' and 'rootfs_1' on NAND flash are os-image
# partitions. These partitions are called as "Bank1/Bank2" in U-Boot
# on WXR-2533DHP, and they are checked conditions when booting.
# Then, U-Boot checks kernel and rootfs volumes in ubi, but U-Boot
# needs "ubi_rootfs" as rootfs volume name. And, U-Boot checks the
# checksum at the end of rootfs (ubi_rootfs).
# When U-Boot writes os-image into the Bank, only kernel, rootfs
# (ubi_rootfs) and rootfs_data (ubi_rootfs_data) volumes are wrote
# into the Bank. (not full ubi image)
#
# == U-Boot Behaviors ==
# - Bank1/Bank2 images are good, images are different
#   -> writes os-image to Bank1 from Bank2
#      (this behavior is used to firmware upgrade in stock firmware)
# - Bank1 image is broken (or checksum error)
#   -> writes os-image to Bank1 from Bank2
# - Bank2 image is broken (or checksum error)
#   -> writes os-image to Bank2 from Bank1
# - Bank1/Bank2 images are broken (or checksum error)
#   -> start tftp
CI_BUF_UBIPART="ubi"
CI_BUF_UBIPART2="rootfs_1"
KERN_VOLNAME="kernel"

buffalo_upgrade_prepare_ubi() {
	local rootfs_length="$1"

	# search first ubi partition
	local mtdnum="$( find_mtd_index "$CI_BUF_UBIPART" )"
	if [ ! "$mtdnum" ]; then
		echo "cannot find first ubi mtd partition $CI_BUF_UBIPART"
		return 1
	fi

	# search second ubi partition
	local mtdnum2="$( find_mtd_index "$CI_BUF_UBIPART2" )"
	if [ ! "$mtdnum2" ]; then
		echo "cannot find second ubi mtd partition $CI_BUF_UBIPART2"
	fi

	local ubidev="$( nand_find_ubi "$CI_BUF_UBIPART" )"
	if [ ! "$ubidev" ]; then
		ubiattach -m "$mtdnum"
		sync
		ubidev="$( nand_find_ubi "$CI_BUF_UBIPART" )"
	fi

	local ubidev2="$( nand_find_ubi "$CI_BUF_UBIPART2" )"
	if [ ! "$ubidev2" ] && [ -n "$mtdnum2" ]; then
		ubiattach -m "$mtdnum2"
		sync
		ubidev2="$( nand_find_ubi "$CI_BUF_UBIPART2" )"
	fi

	if [ ! "$ubidev" ]; then
		ubiformat /dev/mtd$mtdnum -y
		ubiattach -m "$mtdnum"
		sync
		ubidev="$( nand_find_ubi "$CI_BUF_UBIPART" )"
	fi

	local kern_ubivol="$( nand_find_volume $ubidev $KERN_VOLNAME )"
	local root_ubivol="$( nand_find_volume $ubidev ubi_rootfs )"
	local data_ubivol="$( nand_find_volume $ubidev rootfs_data )"
	# Buffalo volumes
	local buf_data_ubivol="$( nand_find_volume $ubidev ubi_rootfs_data )"
	local buf_kern2_ubivol="$( nand_find_volume $ubidev2 $KERN_VOLNAME )"

	# remove ubiblock device of rootfs
	local root_ubiblk="ubiblock${root_ubivol:3}"
	if [ "$root_ubivol" -a -e "/dev/$root_ubiblk" ]; then
		echo "removing $root_ubiblk"
		if ! ubiblock -r /dev/$root_ubivol; then
			echo "cannot remove $root_ubiblk"
			return 1;
		fi
	fi

	# kill volumes
	[ "$kern_ubivol" ] && ubirmvol /dev/$ubidev -N $KERN_VOLNAME || true
	[ "$root_ubivol" ] && ubirmvol /dev/$ubidev -N ubi_rootfs || true
	[ "$data_ubivol" ] && ubirmvol /dev/$ubidev -N rootfs_data || true
	[ "$buf_data_ubivol" ] && ubirmvol /dev/$ubidev -N ubi_rootfs_data || true

	if ! ubimkvol /dev/$ubidev -N $KERN_VOLNAME -s $kernel_length; then
		echo "cannot create kernel volume"
		return 1;
	fi

	if ! ubimkvol /dev/$ubidev -N ubi_rootfs -s $rootfs_length; then
		echo "cannot create rootfs volume"
		return 1;
	fi

	if ! ubimkvol /dev/$ubidev -N rootfs_data -m; then
		echo "cannot initialize rootfs_data volume"
		return 1
	fi

	# remove kernel volume from second ubi partition for U-Boot
	# U-Boot writes os-image into Bank2 from Bank1
	[ "$buf_kern2_ubivol" ] && ubirmvol /dev/$ubidev2 -N $KERN_VOLNAME || true

	sync
	return 0
}

buffalo_do_upgrade() {
	local tar_file="$1"
	local kernel_mtd="$(find_mtd_index $CI_BUF_UBIPART)"

	local board_dir=$(tar tf $tar_file | grep -m 1 '^sysupgrade-.*/$')
	board_dir=${board_dir%/}

	local kernel_length=`(tar xf $tar_file ${board_dir}/kernel -O | wc -c) 2> /dev/null`
	local rootfs_length=`(tar xf $tar_file ${board_dir}/root -O | wc -c) 2> /dev/null`

	local rootfs_type="$(identify_tar "$tar_file" ${board_dir}/root)"

	buffalo_upgrade_prepare_ubi "$rootfs_length" || exit 1

	local ubidev="$( nand_find_ubi "$CI_BUF_UBIPART" )"
	local kern_ubivol="$(nand_find_volume $ubidev $KERN_VOLNAME)"
	tar xf $tar_file ${board_dir}/kernel -O | \
		ubiupdatevol /dev/$kern_ubivol -s $kernel_length -

	local root_ubivol="$(nand_find_volume $ubidev ubi_rootfs)"
	tar xf $tar_file ${board_dir}/root -O | \
		ubiupdatevol /dev/$root_ubivol -s $rootfs_length -

	CI_UBIPART="$CI_BUF_UBIPART"
	nand_do_upgrade_success
}
