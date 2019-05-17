#
# Copyright (C) 2014 OpenWrt.org
#

platform_get_rootfs() {
	local rootfsdev

	if read cmdline < /proc/cmdline; then
		case "$cmdline" in
			*block2mtd=*)
				rootfsdev="${cmdline##*block2mtd=}"
				rootfsdev="${rootfsdev%%,*}"
			;;
			*root=*)
				rootfsdev="${cmdline##*root=}"
				rootfsdev="${rootfsdev%% *}"
			;;
		esac

		echo "${rootfsdev}"
	fi
}

platform_copy_config() {
	case "$(board_name)" in
	erlite)
		mount -t vfat /dev/sda1 /mnt
		cp -af "$CONF_TAR" /mnt/
		umount /mnt
		;;
	esac
}

platform_do_flash() {
	local tar_file=$1
	local board=$2
	local kernel=$3
	local rootfs=$4

	mkdir -p /boot
	mount -t vfat /dev/$kernel /boot

	[ -f /boot/vmlinux.64 -a ! -L /boot/vmlinux.64 ] && {
		mv /boot/vmlinux.64 /boot/vmlinux.64.previous
		mv /boot/vmlinux.64.md5 /boot/vmlinux.64.md5.previous
	}

	echo "flashing kernel to /dev/$kernel"
	tar xf $tar_file sysupgrade-$board/kernel -O > /boot/vmlinux.64
	md5sum /boot/vmlinux.64 | cut -f1 -d " " > /boot/vmlinux.64.md5
	echo "flashing rootfs to ${rootfs}"
	tar xf $tar_file sysupgrade-$board/root -O | dd of="${rootfs}" bs=4096
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
	er)
		kernel=mmcblk0p1
		;;
	erlite)
		kernel=sda1
		;;
	*)
		return 1
	esac

	platform_do_flash $tar_file $board $kernel $rootfs

	return 0
	
}

platform_check_image() {
	local board=$(board_name)

	case "$board" in
	er | \
	erlite)
		local tar_file="$1"
		local kernel_length=`(tar xf $tar_file sysupgrade-$board/kernel -O | wc -c) 2> /dev/null`
		local rootfs_length=`(tar xf $tar_file sysupgrade-$board/root -O | wc -c) 2> /dev/null`
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
