#
# Copyright (C) 2021 OpenWrt.org
#

if [ -x /usr/sbin/blkid ]; then
  RAMFS_COPY_BIN="/usr/sbin/blkid"
fi

platform_get_rootfs() {
	local rootfsdev
	local rootpartuuid

	if read cmdline < /proc/cmdline; then
		case "$cmdline" in
			*root=PARTUUID=*)
				rootpartuuid="${cmdline##*root=PARTUUID=}"
				rootpartuuid="${rootpartuuid%% *}"
				rootfsdev="$(blkid -o device -t PARTUUID="${rootpartuuid}")"
			;;
			*root=*)
				rootfsdev="${cmdline##*root=}"
				rootfsdev="${rootfsdev%% *}"
			;;
		esac

		echo "${rootfsdev}"
	fi
}

platform_get_n821_disk() {
	local partnum=$1
	local DEVNAME
	while read line; do
		export -n "${line}"
	done < $(find /sys/bus/platform/devices/16f0000000000.ehci/ -path \*block/sd[a-z]/uevent)
	echo "/dev/${DEVNAME}${partnum}"
}

platform_copy_config_helper() {
	local device=$1
	local fstype=$2

	mount -t "${fstype}" "$device" /mnt
	cp -af "$UPGRADE_BACKUP" "/mnt/$BACKUP_FILE"
	umount /mnt
}

platform_copy_config() {
	case "$(board_name)" in
	ubnt,erlite|\
	ubnt,usg)
		platform_copy_config_helper /dev/sda1 vfat
		;;
	itus,shield-router)
		platform_copy_config_helper /dev/mmcblk1p1 vfat
		;;
	er|\
	ubnt,edgerouter-4|\
	ubnt,edgerouter-6p)
		platform_copy_config_helper /dev/mmcblk0p1 vfat
		;;
	cisco,vedge1000)
		platform_copy_config_helper "$(platform_get_n821_disk 1)" ext2
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
		if [ "${board}" = "cisco,vedge1000" ]; then
			local rootpartuuid
			rootpartuuid="$(/usr/sbin/blkid -o value -s PARTUUID "${rootfs}")"
			if [ -n "${rootpartuuid}" ]; then
				echo "setting root partition to PARTUUID=${rootpartuuid}"
				fw_setenv bootcmd 'usb start; ext2load usb 0:1 $loadaddr vmlinux.64; bootoctlinux $loadaddr coremask=f endbootargs rootfstype=squashfs rootwait root=PARTUUID='"${rootpartuuid}"
			else
				echo "WARNING: unable to figure out root partition UUID, leaving bootcmd unchanged"
			fi
			mount -t ext2 "${kernel}" /boot
		else
			mount -t vfat "${kernel}" /boot
		fi

		[ -f /boot/vmlinux.64 -a ! -L /boot/vmlinux.64 ] && {
			mv /boot/vmlinux.64 /boot/vmlinux.64.previous
			mv /boot/vmlinux.64.md5 /boot/vmlinux.64.md5.previous
		}

		echo "flashing kernel to $(awk '/\/boot/ {print $1}' /proc/mounts)"
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

	if [ ! -b "${rootfs}" ] && [ "${board}" = "cisco,vedge1000" ]; then
		# Default to the built-in USB disk for N821
		rootfs="$(platform_get_n821_disk 2)"
	fi
	[ -b "${rootfs}" ] || return 1
	case "$board" in
	er | \
	ubnt,edgerouter-4 | \
	ubnt,edgerouter-6p)
		kernel=/dev/mmcblk0p1
		;;
	ubnt,erlite|\
	ubnt,usg)
		kernel=/dev/sda1
		;;
	itus,shield-router)
		kernel=ItusrouterImage
		;;
	cisco,vedge1000)
		kernel="$(platform_get_n821_disk 1)"
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
	itus,shield-router | \
	ubnt,edgerouter-4 | \
	ubnt,edgerouter-6p | \
	ubnt,erlite | \
	ubnt,usg | \
	cisco,vedge1000)
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
