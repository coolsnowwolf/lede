#
# Copyright 2015-2019 Traverse Technologies
# Copyright 2020 NXP
#

RAMFS_COPY_BIN="/usr/sbin/fw_printenv /usr/sbin/fw_setenv /usr/sbin/ubinfo /bin/echo"
RAMFS_COPY_DATA="/etc/fw_env.config /var/lock/fw_printenv.lock"

REQUIRE_IMAGE_METADATA=1

platform_check_image_sdboot() {
	local diskdev partdev diff

	export_bootdevice && export_partdevice diskdev 0 || {
		echo "Unable to determine upgrade device"
		return 1
	}

	# get partitions table from boot device
	get_partitions "/dev/$diskdev" bootdisk

	# get partitions table from sysupgrade.bin
	dd if="$1" of=/tmp/image.bs bs=512b count=1 > /dev/null 2>&1
	sync
	get_partitions /tmp/image.bs image

	# compare tables
	diff="$(grep -F -x -v -f /tmp/partmap.bootdisk /tmp/partmap.image)"

	rm -f /tmp/image.bs /tmp/partmap.bootdisk /tmp/partmap.image

	if [ -n "$diff" ]; then
		echo "Partition layout has changed. Full image will be written."
		ask_bool 0 "Abort" && exit 1
		return 0
	fi
}
platform_do_upgrade_sdboot() {
	local diskdev partdev diff

	export_bootdevice && export_partdevice diskdev 0 || {
		echo "Unable to determine upgrade device"
		return 1
	}

	if [ "$UPGRADE_OPT_SAVE_PARTITIONS" = "1" ]; then
		# get partitions table from boot device
		get_partitions "/dev/$diskdev" bootdisk

		# get partitions table from sysupgrade.bin
		dd if="$1" of=/tmp/image.bs bs=512b count=1 > /dev/null 2>&1
		sync
		get_partitions /tmp/image.bs image

		# compare tables
		diff="$(grep -F -x -v -f /tmp/partmap.bootdisk /tmp/partmap.image)"
	else
		diff=1
	fi

	if [ -n "$diff" ]; then
		dd if="$1" of="/dev/$diskdev" bs=1024 count=4 > /dev/null 2>&1
		dd if="$1" of="$diskdev" bs=1024 skip=4 seek=16384 > /dev/null 2>&1
		sync

		# Separate removal and addtion is necessary; otherwise, partition 1
		# will be missing if it overlaps with the old partition 2
		partx -d - "/dev/$diskdev"
		partx -a - "/dev/$diskdev"

		return 0
	fi

	# write kernel image
	dd if="$1" of="$diskdev" bs=1024 skip=4 seek=16384 count=16384 > /dev/null 2>&1
	sync

	# iterate over each partition from the image and write it to the boot disk
	while read part start size; do
		if export_partdevice partdev $part; then
			echo "Writing image to /dev/$partdev..."
			dd if="$1" of="/dev/$partdev" bs=512 skip="$start" count="$size" > /dev/null 2>&1
			sync
		else
			echo "Unable to find partition $part device, skipped."
		fi
	done < /tmp/partmap.image

}
platform_do_upgrade_traverse_nandubi() {
	bootsys=$(fw_printenv bootsys | awk -F= '{{print $2}}')
	newbootsys=2
	if [ "$bootsys" -eq "2" ]; then
		newbootsys=1
	fi

	# If nand_do_upgrade succeeds, we don't have an opportunity to add any actions of
	# our own, so do it here and set back on failure
	echo "Setting bootsys to #${newbootsys}"
	fw_setenv bootsys $newbootsys
	CI_UBIPART="nandubi"
	CI_KERNPART="kernel${newbootsys}"
	CI_ROOTPART="rootfs${newbootsys}"
	nand_do_upgrade "$1" || (echo "Upgrade failed, setting bootsys ${bootsys}" && fw_setenv bootsys $bootsys)

}
platform_copy_config() {
	local partdev parttype=ext4

	if export_partdevice partdev 1; then
		mount -t $parttype -o rw,noatime "/dev/$partdev" /mnt
		cp -af "$UPGRADE_BACKUP" "/mnt/$BACKUP_FILE"
		umount /mnt
	fi
}
platform_check_image() {
	local board=$(board_name)

	case "$board" in
	traverse,ls1043v | \
	traverse,ls1043s)
		nand_do_platform_check "traverse-ls1043" $1
		return $?
		;;
	fsl,ls1012a-frdm | \
	fsl,ls1012a-rdb | \
	fsl,ls1021a-twr | \
	fsl,ls1043a-rdb | \
	fsl,ls1046a-rdb | \
	fsl,ls1088a-rdb | \
	fsl,ls2088a-rdb)
		return 0
		;;
	fsl,ls1012a-frwy-sdboot | \
	fsl,ls1021a-twr-sdboot | \
	fsl,ls1043a-rdb-sdboot | \
	fsl,ls1046a-rdb-sdboot | \
	fsl,ls1088a-rdb-sdboot)
		platform_check_image_sdboot "$1"
		return 0
		;;
	*)
		echo "Sysupgrade is not currently supported on $board"
		;;
	esac

	return 1
}
platform_do_upgrade() {
	local board=$(board_name)

	# Force the creation of fw_printenv.lock
	mkdir -p /var/lock
	touch /var/lock/fw_printenv.lock

	case "$board" in
	traverse,ls1043v | \
	traverse,ls1043s)
		platform_do_upgrade_traverse_nandubi "$1"
		;;
	fsl,ls1012a-frdm | \
	fsl,ls1012a-rdb | \
	fsl,ls1021a-twr | \
	fsl,ls1043a-rdb | \
	fsl,ls1046a-rdb | \
	fsl,ls1088a-rdb | \
	fsl,ls2088a-rdb)
		PART_NAME=firmware
		default_do_upgrade "$1"
		;;
	fsl,ls1012a-frwy-sdboot | \
	fsl,ls1021a-twr-sdboot | \
	fsl,ls1043a-rdb-sdboot | \
	fsl,ls1046a-rdb-sdboot | \
	fsl,ls1088a-rdb-sdboot)
		platform_do_upgrade_sdboot "$1"
		return 0
		;;
	*)
		echo "Sysupgrade is not currently supported on $board"
		;;
	esac
}
