platform_do_upgrade_emmc() {
	local board=$(board_name)
	local diskdev partdev

	export_bootdevice && export_partdevice diskdev 0 || {
		v "Unable to determine upgrade device"
		return 1
	}
	sync
	if [ "$UPGRADE_OPT_SAVE_PARTITIONS" = "1" ]; then
		get_partitions "/dev/$diskdev" bootdisk
		v "Extract boot sector from the image"
		get_image_dd "$1" of=/tmp/image.bs count=1 bs=512b
		get_partitions /tmp/image.bs image
	fi

	#iterate over each partition from the image and write it to the boot disk
	while read part start size; do
		if export_partdevice partdev $part; then
			if [ "$partdev" = "mmcblk0p2" ]; then
				v "Writing image mmcblk0p3 for /dev/$partdev  $start $size"
				get_image_dd "$1" of="/dev/mmcblk0p3" ibs="512" obs=1M skip="$start" count="$size" conv=fsync
			elif [ "$partdev" = "mmcblk0p1" ]; then
				v "Writing image mmcblk0p1 for /dev/$partdev $start $size"
				get_image_dd "$1" of="/dev/$partdev" ibs="512" obs=1M skip="$start" count="$size" conv=fsync
			fi
		else
			v "Unable to find partition $part device, skipped."
		fi
	done < /tmp/partmap.image

	v "Writing new UUID to /dev/$diskdev..."
	get_image_dd "$1" of="/dev/$diskdev" bs=1 skip=440 count=4 seek=440 conv=fsync
	
	mkfs.ext4 -F -L rootfs_data $(find_mmc_part rootfs_data)

	sleep 1
}
