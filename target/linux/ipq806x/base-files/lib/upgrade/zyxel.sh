#
# Copyright (C) 2016 lede-project.org
#

zyxel_get_rootfs() {
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

zyxel_do_flash() {
	local tar_file=$1
	local kernel=$2
	local rootfs=$3

	# keep sure its unbound
	losetup --detach-all || {
		echo Failed to detach all loop devices. Skip this try.
		reboot -f
	}

	# use the first found directory in the tar archive
	local board_dir=$(tar tf $tar_file | grep -m 1 '^sysupgrade-.*/$')
	board_dir=${board_dir%/}

	echo "flashing kernel to /dev/${kernel}"
	tar xf $tar_file ${board_dir}/kernel -O >/dev/$kernel

	echo "flashing rootfs to ${rootfs}"
	tar xf $tar_file ${board_dir}/root -O >"${rootfs}"

	# a padded rootfs is needed for overlay fs creation
	local offset=$(tar xf $tar_file ${board_dir}/root -O | wc -c)
	[ $offset -lt 65536 ] && {
		echo Wrong size for rootfs: $offset
		sleep 10
		reboot -f
	}

	# Mount loop for rootfs_data
	losetup -o $offset /dev/loop0 "${rootfs}" || {
		echo "Failed to mount looped rootfs_data."
		sleep 10
		reboot -f
	}

	echo "Format new rootfs_data at position ${offset}."
	mkfs.ext4 -F -L rootfs_data /dev/loop0
	mkdir /tmp/new_root
	mount -t ext4 /dev/loop0 /tmp/new_root && {
		echo "Saving config to rootfs_data at position ${offset}."
		cp -v /tmp/sysupgrade.tgz /tmp/new_root/
		umount /tmp/new_root
	}

	# Cleanup
	losetup -d /dev/loop0 >/dev/null 2>&1
	sync
	umount -a
	reboot -f
}

zyxel_do_upgrade() {
	local tar_file="$1"
	local board=$(board_name)
	local rootfs="$(zyxel_get_rootfs)"
	local kernel=

	[ -b "${rootfs}" ] || return 1
	case "$board" in
	zyxel,nbg6817)
		case "$rootfs" in
			"/dev/mmcblk0p5")
				kernel=mmcblk0p4
			;;
			"/dev/mmcblk0p8")
				kernel=mmcblk0p7
			;;
			*)
				return 1
			;;
		esac
		;;
	*)
		return 1
		;;
	esac

	zyxel_do_flash $tar_file $kernel $rootfs

	return 0
}
