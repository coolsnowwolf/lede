#
# Copyright (C) 2015 OpenWrt.org
#

. /lib/functions.sh
#Note: this code also uses some functions from nand.sh, but it is expected to be run by nand.sh, so we are not
#sourcing it explicitly here

UBNT_ERX_KERNEL_INDEX_OFFSET=160

ubnt_get_target_kernel() {
	local factory_mtd=$1
	local current_kernel_index=$(hexdump -s $UBNT_ERX_KERNEL_INDEX_OFFSET -n 1 -e '/1 "%X "' ${factory_mtd})

	if [ $current_kernel_index == "0" ]; then
		echo 'kernel2'
	elif [ $current_kernel_index == "1" ]; then
		echo 'kernel1'
	fi
}

ubnt_update_target_kernel() {
	local factory_mtd=$1
	local kernel_part=$2

	local new_kernel_index
	if [ $kernel_part == "kernel1" ]; then
		new_kernel_index="\x00"
	elif [ $kernel_part == "kernel2" ]; then
		new_kernel_index="\x01"
	else
		echo 'Unknown kernel image index' >&2
		return 1
	fi

	if ! (echo -e $new_kernel_index | dd of=${factory_mtd} bs=1 count=1 seek=$UBNT_ERX_KERNEL_INDEX_OFFSET); then
		echo 'Failed to update kernel bootup index' >&2
		return 1
	fi
}

platform_upgrade_ubnt_erx() {
	local factory_mtd=$(find_mtd_part factory)
	if [ -z "$factory_mtd" ]; then
		echo "cannot find factory partition" >&2
		exit 1
	fi

	local kernel_part="$(ubnt_get_target_kernel ${factory_mtd})"
	if [ -z "$kernel_part" ]; then
		echo "cannot find factory partition" >&2
		exit 1
	fi

	# This is a global defined in nand.sh, sets partition kernel will be flashed into
	CI_KERNPART=${kernel_part}

	#Remove volume possibly left over from stock firmware
	local ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	if [ -z "$ubidev" ]; then
		local mtdnum="$( find_mtd_index "$CI_UBIPART" )"
		if [ -z "$mtdnum" ]; then
			echo "cannot find ubi mtd partition $CI_UBIPART" >&2
			exit 1
		fi
		ubiattach -m "$mtdnum"
		sync
		ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	fi
	if [ -n "$ubidev" ]; then
		local troot_ubivol="$( nand_find_volume $ubidev troot )"
		[ -n "$troot_ubivol" ] && ubirmvol /dev/$ubidev -N troot || true
	fi

	ubnt_update_target_kernel ${factory_mtd} ${kernel_part} || exit 1
}
