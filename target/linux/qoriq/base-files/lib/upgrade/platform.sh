# SPDX-License-Identifier: GPL-2.0-or-later

PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1

platform_get_rootfs_device() {
    local majmin sys path

    majmin="$(awk '$5=="/rom"{print $3; exit}' /proc/self/mountinfo 2>/dev/null)"
    [ -n "$majmin" ] || {
		echo "Unable to determine upgrade device"
		return 1
	}

    sys="/sys/dev/block/$majmin"
    [ -e "$sys" ] || {
		echo "Unable to determine upgrade device"
		return 1
	}

    path="$(readlink -f "$sys" 2>/dev/null)" || return 1
    basename "$path"
}

platform_copy_config_sdboot() {
	local diskdev partdev parttype=ext4

	export_bootdevice && export_partdevice diskdev 0 || {
		echo "Unable to determine upgrade device"
		return 1
	}

	if export_partdevice partdev 1; then
		mount -t $parttype -o rw,noatime "/dev/$partdev" /mnt 2>&1
		echo "Saving config backup..."
		cp -af "$UPGRADE_BACKUP" "/mnt/$BACKUP_FILE"
		umount /mnt
	fi
}

platform_do_upgrade_sdboot() {
	local diskdev partdev parttype=ext4
	local tar_file="$1"
	local board_dir=$(tar tf $tar_file | grep -m 1 '^sysupgrade-.*/$')
	board_dir=${board_dir%/}

	export_bootdevice && export_partdevice diskdev 0 || {
		echo "Unable to determine upgrade device"
		return 1
	}

	if export_partdevice partdev 1; then
		mount -t $parttype -o rw,noatime "/dev/$partdev" /mnt 2>&1
		echo "Writing kernel..."
		tar xf $tar_file ${board_dir}/kernel -O > /mnt/fitImage
		umount /mnt
	fi

	echo "Erasing rootfs..."
	dd if=/dev/zero of=/dev/mmcblk0p2 bs=1M > /dev/null 2>&1
	echo "Writing rootfs..."
	tar xf $tar_file ${board_dir}/root -O  | dd of=/dev/mmcblk0p2 bs=512k > /dev/null 2>&1

}

platform_do_upgrade_nor() {
	local diskdev partdev parttype=ext4
	local tar_file="$1"
	local board_dir=$(tar tf $tar_file | grep -m 1 '^sysupgrade-.*/$')
	board_dir=${board_dir%/}
	local temp_sysupgrade="/tmp/sysupgrade.bin"

	tar xf $tar_file ${board_dir}/kernel -O  > $temp_sysupgrade.tmp || return 1
	tar xf $tar_file ${board_dir}/root -O >> $temp_sysupgrade.tmp  || return 1
	dd if=$temp_sysupgrade.tmp of=$temp_sysupgrade bs=128k conv=sync > /dev/null 2>&1 || return 1
	printf '\xDE\xAD\xC0\xDE' | dd of=$temp_sysupgrade bs=128k oflag=append conv=notrunc,sync > /dev/null 2>&1 || return 1

	default_do_upgrade "$temp_sysupgrade"
}

platform_check_image() {
	case "$(board_name)" in
	fsl,T4240RDB)
		T4240_ROOTFS="$(platform_get_rootfs_device)"
		#Keep info about rootfs device for stage-2
		echo "$T4240_ROOTFS" > /tmp/t4240_rootfs_device

		case "$T4240_ROOTFS" in
			mmcblk*)
				export_bootdevice && export_partdevice diskdev 0 || {
					echo "Unable to determine upgrade device"
					return 1
				}
			;;
			mtdblock*)
				return 0
			;;
			*)
				echo "Unable to determine upgrade device"
				return 1
		esac
		;;
	watchguard,firebox-m300)
		legacy_sdcard_check_image "$1"
		;;
	*)
		return 0
		;;
	esac
}

platform_copy_config() {
	case "$(board_name)" in
	fsl,T4240RDB)
		T4240_ROOTFS="$(cat /tmp/t4240_rootfs_device 2>/dev/null)"

		case "$T4240_ROOTFS" in
			mmcblk*)
			platform_copy_config_sdboot
			;;
			*)
			return 0
		esac
		;;
	watchguard,firebox-m300)
		legacy_sdcard_copy_config "$1"
		;;
	*)
		return 0
	esac
}

platform_do_upgrade() {
	case "$(board_name)" in
	fsl,T4240RDB)
		T4240_ROOTFS="$(cat /tmp/t4240_rootfs_device 2>/dev/null)"

		case "$T4240_ROOTFS" in
			mmcblk*)
				platform_do_upgrade_sdboot "$1"
			;;
			mtdblock*)
				platform_do_upgrade_nor "$1"
			;;
			*)
				echo "Unable to determine upgrade device"
				return 1
		esac
		;;
	watchguard,firebox-m300)
		legacy_sdcard_do_upgrade "$1"
		;;
	*)
		default_do_upgrade "$1"
		;;
	esac
}

