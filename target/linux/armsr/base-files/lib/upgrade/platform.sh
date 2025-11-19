# SPDX-License-Identifier: GPL-2.0-or-later

RAMFS_COPY_BIN="/usr/sbin/blkid"

platform_check_image() {
	local board=$(board_name)
	local diskdev partdev diff
	[ "$#" -gt 1 ] && return 1

	v "Board is ${board}"

	export_bootdevice && export_partdevice diskdev 0 || {
		v "platform_check_image: Unable to determine upgrade device"
		return 1
	}

	get_partitions "/dev/$diskdev" bootdisk

	v "Extract boot sector from the image"
	get_image_dd "$1" of=/tmp/image.bs count=63 bs=512b

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

platform_copy_config() {
	local partdev parttype=ext4

	if export_partdevice partdev 1; then
		part_magic_fat "/dev/$partdev" && parttype=vfat
		mount -t $parttype -o rw,noatime "/dev/$partdev" /mnt
		cp -af "$UPGRADE_BACKUP" "/mnt/$BACKUP_FILE"
		umount /mnt
	else
		v "ERROR: Unable to find partition to copy config data to"
	fi

	sleep 5
}

# To avoid writing over any firmware
# files (e.g ubootefi.var or firmware/X/ aka EBBR)
# Copy efi/openwrt and efi/boot from the new image
# to the existing ESP
platform_do_upgrade_efi_system_partition() {
	local image_file=$1
	local target_partdev=$2
	local image_efisp_start=$3
	local image_efisp_size=$4

	v "Updating ESP on ${target_partdev}"
	NEW_ESP_DIR="/mnt/new_esp_loop"
	CUR_ESP_DIR="/mnt/cur_esp"
	mkdir "${NEW_ESP_DIR}"
	mkdir "${CUR_ESP_DIR}"

	get_image_dd "$image_file" of="/tmp/new_efi_sys_part.img" \
		skip="$image_efisp_start" count="$image_efisp_size"

	mount -t vfat -o loop -o ro /tmp/new_efi_sys_part.img "${NEW_ESP_DIR}"
	if [ ! -d "${NEW_ESP_DIR}/efi/boot" ]; then
		v "ERROR: Image does not contain EFI boot files (/efi/boot)"
		return 1
	fi

	mount -t vfat "/dev/$partdev" "${CUR_ESP_DIR}"

	for d in $(find "${NEW_ESP_DIR}/efi/" -mindepth 1 -maxdepth 1 -type d); do
		v "Copying ${d}"
		newdir_bname=$(basename "${d}")
		rm -rf "${CUR_ESP_DIR}/efi/${newdir_bname}"
		cp -r "${d}" "${CUR_ESP_DIR}/efi"
	done

	umount "${NEW_ESP_DIR}"
	umount "${CUR_ESP_DIR}"
}

platform_do_upgrade() {
	local board=$(board_name)
	local diskdev partdev diff

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

	# Only change the partition table if sysupgrade -p is set,
	# otherwise doing so could interfere with embedded "single storage"
	# (e.g SoC boot from SD card) setups, as well as other user
	# created storage (like uvol)
	if [ -n "$diff" ] && [ "${UPGRADE_OPT_SAVE_PARTITIONS}" = "0" ]; then
		# Need to remove partitions before dd, otherwise the partitions
		# that are added after will have minor numbers offset
		partx -d - "/dev/$diskdev"

		get_image_dd "$1" of="/dev/$diskdev" bs=4096 conv=fsync

		# Separate removal and addtion is necessary; otherwise, partition 1
		# will be missing if it overlaps with the old partition 2
		partx -a - "/dev/$diskdev"

		return 0
	fi

	#iterate over each partition from the image and write it to the boot disk
	while read part start size; do
		if export_partdevice partdev $part; then
			v "Writing image to /dev/$partdev..."
			if [ "$part" = "1" ]; then
				platform_do_upgrade_efi_system_partition \
					$1 $partdev $start $size || return 1
			else
				v "Normal partition, doing DD"
				get_image_dd "$1" of="/dev/$partdev" ibs=512 obs=1M skip="$start" \
					count="$size" conv=fsync
			fi
		else
			v "Unable to find partition $part device, skipped."
		fi
	done < /tmp/partmap.image

	local parttype=ext4

	if (blkid > /dev/null) && export_partdevice partdev 1; then
		part_magic_fat "/dev/$partdev" && parttype=vfat
		mount -t $parttype -o rw,noatime "/dev/$partdev" /mnt
		if export_partdevice partdev 2; then
			THIS_PART_BLKID=$(blkid -o value -s PARTUUID "/dev/${partdev}")
			v "Setting rootfs PARTUUID=${THIS_PART_BLKID}"
			sed -i "s/\(PARTUUID=\)[a-f0-9-]\+/\1${THIS_PART_BLKID}/ig" \
				/mnt/efi/openwrt/grub.cfg
		fi
		umount /mnt
	fi
	# Provide time for the storage medium to flush before system reset
	# (despite the sync/umount it appears NVMe etc. do it in the background)
	sleep 5
}
