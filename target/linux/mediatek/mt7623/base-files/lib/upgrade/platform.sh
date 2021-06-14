
REQUIRE_IMAGE_METADATA=1

# Full system upgrade including preloader for MediaTek SoCs on eMMC or SD
mtk_mmc_full_upgrade() {
	local diskdev partdev diff oldrecovery

	if grep -q root=/dev/mmcblk0p2 /proc/cmdline; then
	    oldrecovery=1
	else
	    oldrecovery=2
	fi

	export_bootdevice && export_partdevice diskdev 0 || {
		echo "Unable to determine upgrade device"
		return 1
	}

	#Keep the persistent random mac address (if it exists)
	mkdir -p /tmp/recovery
	export_partdevice recoverydev $oldrecovery
	if mount -o rw,noatime "/dev/$recoverydev" -tvfat /tmp/recovery; then
		[ -f "/tmp/recovery/mac_addr" ] && cp /tmp/recovery/mac_addr /tmp/
		umount /tmp/recovery
	fi
	sync

	if [ "$SAVE_PARTITIONS" = "1" ]; then
		get_partitions "/dev/$diskdev" bootdisk

		#extract the boot sector from the image
		get_image "$@" | dd of=/tmp/image.bs count=1 bs=512b

		get_partitions /tmp/image.bs image

		#compare tables
		diff="$(grep -F -x -v -f /tmp/partmap.bootdisk /tmp/partmap.image)"
	else
		diff=1
	fi

	if [ -n "$diff" ]; then
		get_image "$@" | dd of="/dev/$diskdev" bs=4096 conv=fsync

		# Separate removal and addition is necessary; otherwise, partition 1
		# will be missing if it overlaps with the old partition 2
		partx -d - "/dev/$diskdev"
		partx -a - "/dev/$diskdev"
	else
		# iterate over each partition from the image and write it to the boot disk
		while read part start size; do
			part="$(($part - 2))"
			if export_partdevice partdev $part; then
				echo "Writing image to /dev/$partdev..."
				get_image "$@" | dd of="/dev/$partdev" ibs="512" obs=1M skip="$start" count="$size" conv=fsync
			else
			    echo "Unable to find partition $part device, skipped."
			fi
		done < /tmp/partmap.image

		#copy partition uuid
		echo "Writing new UUID to /dev/$diskdev..."
		get_image "$@" | dd of="/dev/$diskdev" bs=1 skip=440 count=4 seek=440 conv=fsync
	fi

	export_partdevice recoverydev 2
	if mount -o rw,noatime "/dev/$recoverydev" -t vfat /tmp/recovery; then
		[ -f "/tmp/mac_addr" ] && cp /tmp/mac_addr /tmp/recovery

		if [ "$diskdev" = "mmcblk0" -a -r /tmp/recovery/eMMCboot.bin ]; then
			echo 0 > /sys/block/mmcblk0boot0/force_ro
			dd if=/tmp/recovery/eMMCboot.bin of=/dev/mmcblk0boot0 conv=fsync
			sync
			echo 1 > /sys/block/mmcblk0boot0/force_ro
		fi
		sync
		umount /tmp/recovery
	fi
}

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	bananapi,bpi-r2)
		mtk_mmc_full_upgrade "$1"
		;;

	unielec,u7623-02-emmc-512m)
		local magic="$(get_magic_long "$1")"
		if [ "$magic" = "53444d4d" ]; then
			mtk_mmc_full_upgrade "$1"
		else # Old partial image starting with uImage
			# Keep the persistent random mac address (if it exists)
			recoverydev=mmcblk0p1
			mkdir -p /tmp/recovery
			mount -o rw,noatime /dev/$recoverydev /tmp/recovery
			[ -f "/tmp/recovery/mac_addr" ] && \
				mv -f /tmp/recovery/mac_addr /tmp/
			umount /tmp/recovery

			# 1310720 is the offset in bytes from the start of eMMC and to
			# the location of the kernel (2560 512 byte sectors)
			get_image "$1" | dd of=/dev/mmcblk0 bs=1310720 seek=1 conv=fsync

			mount -o rw,noatime /dev/$recoverydev /tmp/recovery
			[ -f "/tmp/mac_addr" ] && mv -f /tmp/mac_addr /tmp/recovery
			sync
			umount /tmp/recovery
		fi
		;;
	*)
		default_do_upgrade "$1"
		;;
	esac
}

PART_NAME=firmware

platform_check_image() {
	local board=$(board_name)
	local magic="$(get_magic_long "$1")"
	local diskdev partdev diff

	[ "$#" -gt 1 ] && return 1

	case "$board" in
	bananapi,bpi-r2)
		[ "$magic" != "53444d4d" ] && {
			echo "Invalid image type."
			return 1
		}
		export_bootdevice && export_partdevice diskdev 0 || {
			echo "Unable to determine upgrade device"
			return 1
		    }

		get_partitions "/dev/$diskdev" bootdisk

		#extract the boot sector from the image
		get_image "$@" | dd of=/tmp/image.bs count=1 bs=512b 2>/dev/null

		get_partitions /tmp/image.bs image

		#compare tables
		diff="$(grep -F -x -v -f /tmp/partmap.bootdisk /tmp/partmap.image)"

		rm -f /tmp/image.bs /tmp/partmap.bootdisk /tmp/partmap.image

		if [ -n "$diff" ]; then
			echo "Partition layout has changed. Full image will be written."
			ask_bool 0 "Abort" && exit 1
			return 0
		fi
		;;
	unielec,u7623-02-emmc-512m)
		# Can always upgrade to the new-style full image
		[ "$magic" = "53444d4d" ] && return 0

		# Legacy uImage directly at 0xA00 on the eMMC.
		[ "$magic" != "27051956" ] && {
			echo "Invalid image type."
			return 1
		}
		rootpart=$(cat /proc/cmdline)
		rootpart="${rootpart##*root=}"
		rootpart="${rootpart%% *}"
		[ "$rootpart" != "/dev/mmcblk0p2" ] && {
			echo "Cannot downgrade to legacy image."
			return 1
		}
		return 0
		;;
	*)
		echo "Sysupgrade is not supported on your board yet."
		return 1
		;;
	esac

	return 0
}

platform_copy_config() {
	case "$(board_name)" in
	bananapi,bpi-r2|\
	unielec,u7623-02-emmc-512m)
		# platform_do_upgrade() will have set $recoverydev
		if [ -n "$recoverydev" ]; then
			mkdir -p /tmp/recovery
			mount -o rw,noatime "/dev/$recoverydev" -t vfat /tmp/recovery
			cp -af "$CONF_TAR" "/tmp/recovery/sysupgrade.tgz"
			sync
			umount /tmp/recovery
		fi
		;;
	esac
}
