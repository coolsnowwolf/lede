legacy_sdcard_check_image() {
	local file="$1"
	local diskdev partdev diff

	export_bootdevice && export_partdevice diskdev 0 || {
		v "Unable to determine upgrade device"
	return 1
	}

	get_partitions "/dev/$diskdev" bootdisk

	v "Extract boot sector from the image"
	get_image_dd "$1" of=/tmp/image.bs count=1 bs=512b

	get_partitions /tmp/image.bs image

	#compare tables
	diff="$(grep -F -x -v -f /tmp/partmap.bootdisk /tmp/partmap.image)"

	rm -f /tmp/image.bs /tmp/partmap.bootdisk /tmp/partmap.image

	if [ -n "$diff" ]; then
		v "Partition layout has changed. Full image will be written."
		ask_bool 0 "Abort" && exit 1
		return 0
	fi
}

legacy_sdcard_do_upgrade() {
	local board=$(board_name)
	local diskdev partdev diff

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

		#compare tables
		diff="$(grep -F -x -v -f /tmp/partmap.bootdisk /tmp/partmap.image)"
	else
		diff=1
	fi

	if [ -n "$diff" ]; then
		get_image_dd "$1" of="/dev/$diskdev" bs=4096 conv=fsync

		# Separate removal and addtion is necessary; otherwise, partition 1
		# will be missing if it overlaps with the old partition 2
		partx -d - "/dev/$diskdev"
		partx -a - "/dev/$diskdev"
	else
		v "Writing bootloader to /dev/$diskdev"
		get_image_dd "$1" of="$diskdev" bs=512 skip=1 seek=1 count=2048 conv=fsync
		#iterate over each partition from the image and write it to the boot disk
		while read part start size; do
			if export_partdevice partdev $part; then
				v "Writing image to /dev/$partdev..."
				get_image_dd "$1" of="/dev/$partdev" ibs="512" obs=1M skip="$start" count="$size" conv=fsync
			else
				v "Unable to find partition $part device, skipped."
			fi
		done < /tmp/partmap.image

		v "Writing new UUID to /dev/$diskdev..."
		get_image_dd "$1" of="/dev/$diskdev" bs=1 skip=440 count=4 seek=440 conv=fsync
	fi

	sleep 1
}

legacy_sdcard_copy_config() {
	local partdev

	if export_partdevice partdev 1; then
		mkdir -p /boot
		[ -f /boot/kernel.img ] || mount -o rw,noatime /dev/$partdev /boot
		cp -af "$UPGRADE_BACKUP" "/boot/$BACKUP_FILE"
		sync
		umount /boot
	fi
}
