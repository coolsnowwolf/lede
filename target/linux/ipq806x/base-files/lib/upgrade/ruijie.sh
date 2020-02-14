#
# Copyright (C) 2016 lede-project.org
# Copyright (C) 2020 AnYun
#

ruijie_do_flash() {
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
	tar xf $tar_file ${board_dir}/kernel -O > $kernel

	echo "flashing rootfs to $rootfs"
	tar xf $tar_file ${board_dir}/root -O > $rootfs

	# Cleanup
	losetup -d /dev/loop0 >/dev/null 2>&1
	sync
	umount -a
	reboot -f
}

ruijie_do_upgrade() {
	local tar_file="$1"
	local board=$(board_name)
	local kernel=
	local rootfs=

	case "$board" in
	ruijie,rg-mtfi-m520)
		kernel="/dev/mmcblk0p2"
		rootfs="/dev/mmcblk0p3"
		;;
	*)
		return 1
		;;
	esac

	ruijie_do_flash $tar_file $kernel $rootfs

	return 0
}
