
platform_check_image() {
	local diskdev partdev diff

	export_bootdevice && export_partdevice diskdev 0 || {
		v "Unable to determine upgrade device"
		return 1
	}
	[ "$SAVE_CONFIG" -eq 1 ] && return 0

	get_partitions "/dev/$diskdev" bootdisk

	#extract the boot sector from the image
	get_image "$@" | dd of=/tmp/image.bs count=1 bs=512b 2>/dev/null

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

platform_do_upgrade() {
	local diskdev partdev part start size cursize

	export_bootdevice && export_partdevice diskdev 0 || {
		v "Unable to determine upgrade device"
		return 1
	}

	sync

	get_partitions "/dev/$diskdev" bootdisk

	#extract the boot sector from the image
	get_image "$@" | dd of=/tmp/image.bs count=1 bs=512b

	get_partitions /tmp/image.bs image

	#iterate over each partition from the image and check boot disk partition size
	while read part start size; do
		if [ -n "$UPGRADE_BACKUP" -a "$part" -ge 3 ]; then
			continue
		fi
		cursize=$(echo $(grep -m1 '^ *'"$part"' *' /tmp/partmap.bootdisk) | cut -d' ' -f 3)
		if [ -z "$cursize" ]; then
			v "Unable to find partition $part on boot disk"
			return 1
		fi
		if [ "$size" -gt "$cursize" ]; then
			v "Partition $part on image is bigger than boot disk ($size > $cursize)"
			return 1
		fi
	done < /tmp/partmap.image

	#iterate over each partition from the image and write it to the boot disk
	while read part start size; do
		if [ -n "$UPGRADE_BACKUP" -a "$part" -ge 3 ]; then
			v "Skip partition $part >= 3 when upgrading"
			continue
		fi
		if export_partdevice partdev $part; then
			v "Writing image to /dev/$partdev..."
			if [ "$part" -eq 3 ]; then
				echo "RESET000" | dd of="/dev/$partdev" bs=512 count=1 conv=sync,fsync 2>/dev/null
			else
				get_image "$@" | dd of="/dev/$partdev" ibs="512" obs=1M skip="$start" count="$size" conv=fsync
			fi
		else
			v "Unable to find partition $part device, skipped."
		fi
	done < /tmp/partmap.image

	#copy partition uuid
	v "Writing new UUID to /dev/$diskdev..."
	get_image "$@" | dd of="/dev/$diskdev" bs=1 skip=440 count=4 seek=440 conv=fsync
}
