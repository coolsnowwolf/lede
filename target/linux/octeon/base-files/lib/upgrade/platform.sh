#
# Copyright (C) 2021 OpenWrt.org
#

platform_get_rootfs() {
	local rootfsdev

	if read cmdline < /proc/cmdline; then
		case "$cmdline" in
			*root=*)
				rootfsdev="${cmdline##*root=}"
				rootfsdev="${rootfsdev%% *}"
			;;
		esac

		echo "${rootfsdev}"
	fi
}

platform_copy_config_helper() {
	local device=$1

	mount -t vfat "$device" /mnt
	cp -af "$CONF_TAR" /mnt/
	umount /mnt
}

platform_copy_config() {
	case "$(board_name)" in
	erlite)
		platform_copy_config_helper /dev/sda1
		;;
	itus,shield-router)
		platform_copy_config_helper /dev/mmcblk1p1
		;;
	ubnt,edgerouter-4|\
	ubnt,edgerouter-6p)
		platform_copy_config_helper /dev/mmcblk0p1
		;;
	esac
}

platform_do_flash() {
	local tar_file=$1
	local board=$2
	local kernel=$3
	local rootfs=$4

	local board_dir=$(tar tf "$tar_file" | grep -m 1 '^sysupgrade-.*/$')
	board_dir=${board_dir%/}
	[ -n "$board_dir" ] || return 1

	mkdir -p /boot

	if [ $board = "itus,shield-router" ]; then
		# mmcblk1p1 (fat) contains all ELF-bin images for the Shield
		mount /dev/mmcblk1p1 /boot

		echo "flashing Itus Kernel to /boot/$kernel (/dev/mmblk1p1)"
		tar -Oxf $tar_file "$board_dir/kernel" > /boot/$kernel
	else
		mount -t vfat /dev/$kernel /boot

		[ -f /boot/vmlinux.64 -a ! -L /boot/vmlinux.64 ] && {
			mv /boot/vmlinux.64 /boot/vmlinux.64.previous
			mv /boot/vmlinux.64.md5 /boot/vmlinux.64.md5.previous
		}

		echo "flashing kernel to /dev/$kernel"
		tar xf $tar_file $board_dir/kernel -O > /boot/vmlinux.64
		md5sum /boot/vmlinux.64 | cut -f1 -d " " > /boot/vmlinux.64.md5
	fi

	echo "flashing rootfs to ${rootfs}"
	tar xf $tar_file $board_dir/root -O | dd of="${rootfs}" bs=4096

	sync
	umount /boot
}

platform_do_upgrade() {
	local tar_file="$1"
	local board=$(board_name)
	local rootfs="$(platform_get_rootfs)"
	local kernel=

	[ -b "${rootfs}" ] || return 1
	case "$board" in
	er | \
	ubnt,edgerouter-4 | \
	ubnt,edgerouter-6p)
		kernel=mmcblk0p1
		;;
	erlite)
		kernel=sda1
		;;
	itus,shield-router)
		kernel=ItusrouterImage
		;;
	*)
		return 1
	esac

	platform_do_flash $tar_file $board $kernel $rootfs

	return 0
}

platform_check_image() {
	local board=$(board_name)
	local tar_file="$1"

	local board_dir=$(tar tf "$tar_file" | grep -m 1 '^sysupgrade-.*/$')
	board_dir=${board_dir%/}
	[ -n "$board_dir" ] || return 1

	case "$board" in
	er | \
	erlite | \
	itus,shield-router | \
	ubnt,edgerouter-4 | \
	ubnt,edgerouter-6p)
		local kernel_length=$(tar xf $tar_file $board_dir/kernel -O | wc -c 2> /dev/null)
		local rootfs_length=$(tar xf $tar_file $board_dir/root -O | wc -c 2> /dev/null)
		[ "$kernel_length" = 0 -o "$rootfs_length" = 0 ] && {
			echo "The upgrade image is corrupt."
			return 1
		}
		return 0
		;;
	esac

	echo "Sysupgrade is not yet supported on $board."
	return 1
}
