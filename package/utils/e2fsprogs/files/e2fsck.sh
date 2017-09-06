#!/bin/sh
# Copyright 2010 Vertical Communications
# Copyright 2012 OpenWrt.org
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

fsck_e2fsck() {
	set -o pipefail
	e2fsck -p "$device" 2>&1 | logger -t "fstab: e2fsck ($device)"
	local status="$?"
	set +o pipefail
	case "$status" in
		0|1) ;; #success
		2) reboot;;
		4) echo "e2fsck ($device): Warning! Uncorrected errors."| logger -t fstab
			return 1
			;;
		*) echo "e2fsck ($device): Error $status. Check not complete."| logger -t fstab;;
	esac
	return 0
}

fsck_ext2() {
	fsck_e2fsck "$@"
}

fsck_ext3() {
	fsck_e2fsck "$@"
}

fsck_ext4() {
	fsck_e2fsck "$@"
}

append libmount_known_fsck "ext2"
append libmount_known_fsck "ext3"
append libmount_known_fsck "ext4"
