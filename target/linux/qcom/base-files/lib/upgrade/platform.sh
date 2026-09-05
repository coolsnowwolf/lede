# SPDX-License-Identifier: GPL-2.0-or-later

RAMFS_COPY_BIN="/usr/sbin/blkid"

qcom_get_logical_block_size() {
	local diskdev="$1"
	local logical_block_size=512

	if [ -n "$diskdev" ]; then
		logical_block_size=$(cat "/sys/class/block/$diskdev/queue/logical_block_size" 2>/dev/null)
	elif [ -n "$DEVNAME" ]; then
		logical_block_size=$(cat "/sys/class/block/$DEVNAME/queue/logical_block_size" 2>/dev/null)
	fi

	[ -n "$logical_block_size" ] || logical_block_size=512
	echo "$logical_block_size"
}

qcom_detect_gpt_sector_size() {
	local disk="$1"
	local magic

	magic=$(dd if="$disk" bs=8 count=1 skip=64 2>/dev/null)
	[ "$magic" = "EFI PART" ] && {
		echo 512
		return
	}

	magic=$(dd if="$disk" bs=8 count=1 skip=512 2>/dev/null)
	[ "$magic" = "EFI PART" ] && {
		echo 4096
		return
	}

	echo 0
}

qcom_get_partitions() { # <device> <filename>
	local disk="$1"
	local filename="$2"

	if [ -b "$disk" -o -f "$disk" ]; then
		v "Reading partition table from $filename..."

		local magic
		magic=$(dd if="$disk" bs=2 count=1 skip=255 2>/dev/null)
		if [ "$magic" != $'\x55\xAA' ]; then
			v "Invalid partition table on $disk"
			return 1
		fi

		rm -f "/tmp/partmap.$filename"

		local part gpt_sector_size factor entry_base entry_offset
		gpt_sector_size="$(qcom_detect_gpt_sector_size "$disk")"

		if [ "$gpt_sector_size" != "0" ]; then
			factor=$((gpt_sector_size / 512))
			entry_base=$((gpt_sector_size * 2))

			for part in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15; do
				entry_offset=$((entry_base + (part - 1) * 0x80))
				set -- $(hexdump -v -n 48 -s "$entry_offset" -e '4/4 "%08x"" "4/4 "%08x"" "4/4 "0x%08X "' "$disk")

				local type="$1"
				local lba="$(( $(hex_le32_to_cpu $4) * 0x100000000 + $(hex_le32_to_cpu $3) ))"
				local end="$(( $(hex_le32_to_cpu $6) * 0x100000000 + $(hex_le32_to_cpu $5) ))"
				local num="$(( $end - $lba + 1 ))"

				[ "$type" = "00000000000000000000000000000000" ] && continue

				printf "%2d %5d %7d\n" $part $((lba * factor)) $((num * factor)) >> "/tmp/partmap.$filename"
			done
		else
			for part in 1 2 3 4; do
				set -- $(hexdump -v -n 12 -s "$((0x1B2 + $part * 16))" -e '3/4 "0x%08X "' "$disk")

				local type="$(( $(hex_le32_to_cpu $1) % 256))"
				local lba="$(( $(hex_le32_to_cpu $2) ))"
				local num="$(( $(hex_le32_to_cpu $3) ))"

				[ $type -gt 0 ] || continue

				printf "%2d %5d %7d\n" $part $lba $num >> "/tmp/partmap.$filename"
			done
		fi
	fi

	return 0
}

qcom_get_image_part_skip() {
	local board="$1"
	local start="$4"

	case "$board" in
	radxa,dragon-q6a|radxa,dragon-q8b)
		# qcom_get_partitions() normalizes GPT LBAs to 512-byte sectors for
		# both classic and 4K-sector images, so the source-image skip stays
		# equal to the normalized start offset.
		;;
	esac

	echo "$start"
}

platform_check_image() {
	local board="$(board_name)"
	local diskdev diff

	[ "$#" -gt 1 ] && return 1

	v "Board is ${board}"

	export_bootdevice && export_partdevice diskdev 0 || {
		v "platform_check_image: Unable to determine upgrade device"
		return 1
	}

	qcom_get_partitions "/dev/$diskdev" bootdisk || return 1

	v "Extract boot sector from the image"
	get_image_dd "$1" of=/tmp/image.bs count=16 bs=4096
	qcom_get_partitions /tmp/image.bs image || {
		rm -f /tmp/image.bs /tmp/partmap.bootdisk /tmp/partmap.image
		return 1
	}

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

platform_do_upgrade_efi_system_partition() {
	local image_file="$1"
	local target_partdev="$2"
	local image_efisp_start="$3"
	local image_efisp_size="$4"
	local target_diskdev="$5"
	local new_esp_dir="/mnt/new_esp_loop"
	local cur_esp_dir="/mnt/cur_esp"
	local new_esp_img="/tmp/new_efi_sys_part.img"
	local logical_block_size=512
	local root_partuuid=""

	v "Updating ESP on ${target_partdev}"

	# UFS-based qcom targets use 4K logical sectors, and the ESP image is
	# a block layer that rejects mounting the pre-upgrade FAT volume due to
	# its 512-byte logical sector size. Update grub.cfg inside the extracted
	# image, then rewrite the whole partition directly.
	logical_block_size="$(qcom_get_logical_block_size "$target_diskdev")"
	if [ "$logical_block_size" = "4096" ]; then
		mkdir -p "${new_esp_dir}"
		get_image_dd "$image_file" of="${new_esp_img}" \
			skip="$image_efisp_start" count="$image_efisp_size" || return 1

		mount -t vfat -o loop "${new_esp_img}" "${new_esp_dir}" || return 1
		if [ ! -d "${new_esp_dir}/efi/boot" ]; then
			umount "${new_esp_dir}"
			v "ERROR: Image does not contain EFI boot files (/efi/boot)"
			return 1
		fi

		if export_partdevice partdev 2; then
			root_partuuid="$(blkid -o value -s PARTUUID "/dev/${partdev}")"
		fi
		v "Setting rootfs PARTUUID=${root_partuuid}"
		[ -n "$root_partuuid" ] && sed -i "s#\(PARTUUID=\)[a-f0-9-]\+#\1${root_partuuid}#ig" \
			"${new_esp_dir}/efi/openwrt/grub.cfg"

		umount "${new_esp_dir}"

		dd if="${new_esp_img}" of="/dev/$target_partdev" bs=1M conv=fsync 2>/dev/null || return 1
		return $?
	fi

	mkdir -p "${new_esp_dir}" "${cur_esp_dir}"

	get_image_dd "$image_file" of="${new_esp_img}" \
		skip="$image_efisp_start" count="$image_efisp_size"

	mount -t vfat -o loop -o ro "${new_esp_img}" "${new_esp_dir}"
	if [ ! -d "${new_esp_dir}/efi/boot" ]; then
		v "ERROR: Image does not contain EFI boot files (/efi/boot)"
		return 1
	fi

	mount -t vfat "/dev/$target_partdev" "${cur_esp_dir}"

	for d in $(find "${new_esp_dir}/efi/" -mindepth 1 -maxdepth 1 -type d); do
		local newdir_bname
		newdir_bname="$(basename "${d}")"
		v "Copying ${d}"
		rm -rf "${cur_esp_dir}/efi/${newdir_bname}"
		cp -r "${d}" "${cur_esp_dir}/efi"
	done

	umount "${new_esp_dir}"
	umount "${cur_esp_dir}"
}

platform_do_upgrade() {
	local board diskdev partdev diff
	local image_skip
	local esp_updated_in_image=0

	board="$(board_name)"
	export_bootdevice && export_partdevice diskdev 0 || {
		v "platform_do_upgrade: Unable to determine upgrade device"
		return 1
	}

	local logical_block_size
	logical_block_size="$(qcom_get_logical_block_size "$diskdev")"

	sync

	if [ "$UPGRADE_OPT_SAVE_PARTITIONS" = "1" ]; then
		qcom_get_partitions "/dev/$diskdev" bootdisk || return 1
		v "Extract boot sector from the image"
		get_image_dd "$1" of=/tmp/image.bs count=16 bs=4096
		qcom_get_partitions /tmp/image.bs image || {
			rm -f /tmp/image.bs /tmp/partmap.bootdisk /tmp/partmap.image
			return 1
		}
		diff="$(grep -F -x -v -f /tmp/partmap.bootdisk /tmp/partmap.image)"
	else
		diff=1
	fi

	if [ -n "$diff" ] && [ "${UPGRADE_OPT_SAVE_PARTITIONS}" = "0" ]; then
		partx -d - "/dev/$diskdev"
		get_image_dd "$1" of="/dev/$diskdev" bs="$logical_block_size" conv=fsync
		partx -a - "/dev/$diskdev"
		return 0
	fi

	while read part start size; do
		if export_partdevice partdev $part; then
				v "Writing image to /dev/$partdev..."
				if [ "$part" = "1" ]; then
						platform_do_upgrade_efi_system_partition \
							"$1" "$partdev" "$start" "$size" "$diskdev" || return 1
						[ "$logical_block_size" = "4096" ] && esp_updated_in_image=1
				else
					image_skip="$(qcom_get_image_part_skip "$board" "$diskdev" "$part" "$start")"
					get_image_dd "$1" of="/dev/$partdev" ibs=512 obs=1M \
						skip="$image_skip" count="$size" conv=fsync
				fi
		else
			v "Unable to find partition $part device, skipped."
		fi
	done < /tmp/partmap.image

	local parttype=ext4

	if [ "$esp_updated_in_image" != "1" ] && (blkid > /dev/null) && export_partdevice partdev 1; then
		part_magic_fat "/dev/$partdev" && parttype=vfat
		mount -t $parttype -o rw,noatime "/dev/$partdev" /mnt
		local this_part_blkid
		if export_partdevice partdev 2; then
			this_part_blkid="$(blkid -o value -s PARTUUID "/dev/${partdev}")"
		else
			this_part_blkid=""
		fi
		v "Setting rootfs PARTUUID=${this_part_blkid}"
		[ -n "$this_part_blkid" ] && sed -i "s#\(PARTUUID=\)[a-f0-9-]\+#\1${this_part_blkid}#ig" \
			/mnt/efi/openwrt/grub.cfg
		umount /mnt
	fi

	sleep 5
}
