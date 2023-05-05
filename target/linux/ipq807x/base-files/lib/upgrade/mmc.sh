#
# Copyright (C) 2016 lede-project.org
#

# this can be used as a generic mmc upgrade script
# just add a device entry in platform.sh, 
# define "kernelname" and "rootfsname" and call mmc_do_upgrade
# after the kernel and rootfs flash a loopdev (as overlay) is 
# setup on top of the rootfs partition
# for the proper function a padded rootfs image is needed, basically 
# append "pad-to 64k" to the image definition
# this is based on the ipq806x zyxel.sh mmc upgrade

. /lib/functions.sh

mmc_do_upgrade() {
	local tar_file="$1"
	local rootfs=
	local kernel=

			[ -z "$kernel" ] && kernel=$(find_mmc_part ${kernelname})
			[ -z "$rootfs" ] && rootfs=$(find_mmc_part ${rootfsname})

			[ -z "$kernel" ] && echo "Upgrade failed: kernel partition not found! Rebooting..." && reboot -f
			[ -z "$rootfs" ] && echo "Upgrade failed: rootfs partition not found! Rebooting..." && reboot -f

	mmc_do_flash $tar_file $kernel $rootfs

	return 0
}

mmc_do_flash() {
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

	echo "flashing kernel to $kernel"
	tar xf $tar_file ${board_dir}/kernel -O >$kernel

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
	local loopdev="$(losetup -f)"
	losetup -o $offset $loopdev $rootfs || {
		echo "Failed to mount looped rootfs_data."
		sleep 10
		reboot -f
	}

	echo "Format new rootfs_data at position ${offset}."
	mkfs.ext4 -F -L rootfs_data $loopdev
	mkdir /tmp/new_root
	mount -t ext4 $loopdev /tmp/new_root && {
		echo "Saving config to rootfs_data at position ${offset}."
		cp -v "$UPGRADE_BACKUP" "/tmp/new_root/$BACKUP_FILE"
		umount /tmp/new_root
	}

	# Cleanup
	losetup -d $loopdev >/dev/null 2>&1
	sync
	umount -a
	reboot -f
}
