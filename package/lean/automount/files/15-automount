#!/bin/sh

# Copyright (C) 2015 OpenWrt.org

# 0 yes blockdevice handles this - 1 no it is not there
blkdev=`dirname $DEVPATH`
basename=`basename $blkdev`
device=`basename $DEVPATH`
skip=`block info | grep -vE '(f2fs|kernel|squashfs)' | sed 's/\(.*\): .*/\1/' | grep -q $device ; echo $?`
path=$DEVPATH

if [ $basename != "block" ] && [ -z "${device##sd*}" ] && [ $skip -eq 1 ]; then
	mntpnt=$device
	case "$ACTION" in
		add)
			mkdir -p /mnt/$mntpnt
			chmod 777 /mnt/$mntpnt
			# Try to be gentle on solid state devices
			mount -o rw,noatime,discard /dev/$device /mnt/$mntpnt
			;;
		remove)
			# Once the device is removed, the /dev entry disappear. We need mountpoint
			mountpoint=`mount |grep /dev/$device | sed 's/.* on \(.*\) type.*/\1/'`
			umount -l $mountpoint
			;;
	esac
fi
