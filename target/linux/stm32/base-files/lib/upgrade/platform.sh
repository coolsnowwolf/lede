REQUIRE_IMAGE_METADATA=1

UBOOT_ENV_PART=3
BOOT_PART=4
ROOTFS_PART=5

RAMFS_COPY_BIN='blockdev'

export_bootdevice() {
	local cmdline uuid blockdev uevent line class
	local MAJOR MINOR DEVNAME DEVTYPE
	local rootpart="$(cmdline_get_var root)"

	case "$rootpart" in
		PARTUUID=????????-????-????-????-??????????0?/PARTNROFF=1 | \
		PARTUUID=????????-????-????-????-??????????05)
			uuid="${rootpart#PARTUUID=}"
			uuid="${uuid%/PARTNROFF=1}"
			uuid="${uuid%0?}00"
			for disk in $(find /dev -type b); do
				set -- $(dd if=$disk bs=1 skip=568 count=16 2>/dev/null | hexdump -v -e '8/1 "%02x "" "2/1 "%02x""-"6/1 "%02x"')
				if [ "$4$3$2$1-$6$5-$8$7-$9" = "$uuid" ]; then
					uevent="/sys/class/block/${disk##*/}/uevent"
					break
				fi
			done
		;;
	esac

	if [ -e "$uevent" ]; then
		while read line; do
			export -n "$line"
		done < "$uevent"
		export BOOTDEV_MAJOR=$MAJOR
		export BOOTDEV_MINOR=$MINOR
		return 0
	fi

	return 1
}

platform_check_image() {
	local diskdev partdev diff

        [ "$#" -gt 1 ] && return 1

	export_bootdevice && export_partdevice diskdev 0 || {
		v "platform_check_image: Unable to determine upgrade device"
		return 1
	}

	get_partitions "/dev/$diskdev" bootdisk

	v "Extract the boot sector from the image"
	get_image_dd "$1" of=/tmp/image.bs count=63 bs=512b

	get_partitions /tmp/image.bs image

	#compare tables
	diff="$(grep -F -x -v -f /tmp/partmap.bootdisk /tmp/partmap.image)"

	rm -f /tmp/image.bs /tmp/partmap.bootdisk /tmp/partmap.image

	if [ -n "$diff" ]; then
		echo "Partition layout has changed. Full image will be written."
		ask_bool 0 "Abort" && exit 1
		return 0
	fi
}

platform_do_upgrade() {
	local diskdev partdev diff partlabel

	export_bootdevice && export_partdevice diskdev 0 || {
		v "platform_do_upgrade: Unable to determine upgrade device"
		return 1
	}

	sync

	if [ "$UPGRADE_OPT_SAVE_PARTITIONS" = "1" ]; then
		get_partitions "/dev/$diskdev" bootdisk

		v "Extract boot sector from the image"
		get_image_dd "$1" of=/tmp/image.bs count=63 bs=512b

		get_partitions /tmp/image.bs image

		#compare tables
		diff="$(grep -F -x -v -f /tmp/partmap.bootdisk /tmp/partmap.image)"
	else
		diff=1
	fi

	if [ -n "$diff" ]; then
		rm -rf /tmp/ubootenv

		if export_partdevice partdev $UBOOT_ENV_PART; then
			v "Saving u-boot env (/dev/$partdev) before to write image"
			get_image_dd "/dev/$partdev" of=/tmp/ubootenv
		fi

		v "Writing image to /dev/$diskdev..."
		get_image_dd "$1" of="/dev/$diskdev" conv=fsync

		blockdev --rereadpt "/dev/$diskdev"

		[ -f /tmp/ubootenv ] && {
			# iterate over each partition from the image to find the
			# u-boot-env partition and restore u-boot env.
			while read part start size; do
				if export_partdevice partdev $part; then
					while read line; do
						eval "local l$line"
					done < "/sys/class/block/$partdev/uevent"

					[ "$lPARTNAME" = "u-boot-env" ] || continue

					v "Writting u-boot env to /dev/$partdev"
					get_image_dd /tmp/ubootenv of="/dev/$partdev" conv=fsync

					return 0
				fi
			done < /tmp/partmap.image
		}
		return 0
	fi

	#iterate over each partition from the image and write it to the boot disk
	while read part start size; do
		if export_partdevice partdev $part; then
			# do not erase u-boot env
			[ "$part" = "$UBOOT_ENV_PART" ] && continue

			v "Writing image to /dev/$partdev..."
			v "Normal partition, doing DD"
			get_image_dd "$1" of="/dev/$partdev" ibs=512 obs=1M skip="$start" \
				count="$size" conv=fsync
		else
			v "Unable to find partition $part device, skipped."
		fi
	done < /tmp/partmap.image

	if export_partdevice partdev "$BOOT_PART"; then
		mount -t ext4 -o rw,noatime "/dev/$partdev" /mnt
		local partuuid="$(cmdline_get_var root)"
		v "Setting rootfs ${partuuid}"
		sed -i "s/PARTUUID=[a-f0-9-]\+/${partuuid}/ig" \
			/mnt/extlinux/extlinux.conf
		umount /mnt
	fi
}

platform_copy_config() {
	local partdev

	# Iterate over each partition from the image to find the boot partition
	# and copy the config tarball.
	# The partlabel is used to find the partition.
	# An hardcoded partition number cannot be used, as it could be wrong if
	# the partition table changed, and the full image was written.
	while read part start size; do
		# config is copied in the boot partition, as for squashfs image, the
		# rootfs partition is not writable.
		if export_partdevice partdev "$part"; then
			while read line; do
				eval "local l$line"
			done < "/sys/class/block/$partdev/uevent"

			[ "$lPARTNAME" = "boot" ] || continue

			mount -t ext4 -o rw,noatime "/dev/$partdev" /mnt
			cp -af "$UPGRADE_BACKUP" "/mnt/$BACKUP_FILE"
			umount /mnt
			return 0
		else
			v "ERROR: Unable to find partition to copy config data to"
		fi
	done < /tmp/partmap.image
}

