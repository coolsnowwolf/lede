#!/bin/sh
# Copy running OpenWrt to eMMC for amlogic

. /lib/functions.sh
. /lib/functions/system.sh

include /lib/upgrade
NEWLINE="
"

function umount_disk() {
	local mounted
	local to="${1#/dev/}"
	umount "/dev/$to"* 2>/dev/null
	mounted="`mount | cut -d' ' -f1 | grep -Fm1 "/dev/$to"`"
	if [ -n "$mounted" ] ; then
		echo "can not umount $mounted" >&2
		return 1
	fi
	return 0
}

function part_disk() {
	local to="${1#/dev/}"
	dd if=/dev/zero of="/dev/$to" bs=512 count=1 conv=notrunc 2>/dev/null
	parted --script "/dev/$to" \
		mktable msdos \
		mkpart primary 68MiB 132MiB \
		mkpart primary 132MiB 388MiB \
		mkpart primary 764MiB 2812MiB
}

function get_bootdisk() {
	local var=$1
	local diskdev
	export_bootdevice && export_partdevice diskdev 0 || {
		echo "Unable to determine upgrade device" >&2
		return 1
	}
	export "$var=$diskdev"
	return 0
}

function get_emmcs() {
	local disk index disks
	local bootdisk=$1

	index=1
    for disk in `cd /sys/block && ls | grep '^mmcblk' | grep -Ev 'mmcblk\d+boot\d+'`; do
        [[ "$bootdisk" = "$disk" ]] && continue
        [ -b /dev/$disk ] && [ "`cat /sys/block/$disk/size`" -gt 0 ] || continue
        local diskinfo="`parted -ms "/dev/$disk" unit GiB print 2>/dev/null | grep -m1 "^/dev/$disk:"`"
        [ -z "$diskinfo" ] && continue
        local size="`echo "$diskinfo" | cut -d: -f2`"
		# check disk greater than 2.8GB
        [ $(( `echo "${size%%GiB}" | sed 's/\.\(.\).*/ * 10 + \1/'` )) -ge 28 ] || continue
        local model="`echo "$diskinfo" | cut -d: -f7 | sed 's/"/_/g'`"
        disks="${disks}$index. $disk $size \"$model\"$NEWLINE"
		index=$(($index + 1))
    done
	echo "$disks"
	return 0
}

function get_emmc() {
	local index
	local var=$2
	local emmcs="$(get_emmcs $1)"
	[ -z "$emmcs" ] && {
		echo "Get eMMC list failed! (Maybe already running on eMMC? Current boot disk is $1)" >&2
		return 1
	}
	local count=$(echo "$emmcs" | wc -l)
	if [ "$count" -gt 1 ]; then
		while true; do
			echo "Found these eMMC devices:"
			echo "Index.  device  size  model"
			echo "$emmcs"
			echo -n "Select a disk to install (input index then press Enter): "
			read index || {
				echo "No input!" >&2
				exit 1
			}
			if [ -z "$index" ]; then
				echo "Empty input!"
			else
				echo "$emmcs" | grep -q "^${index}. " && break
				echo "Wrong index!"
			fi
		done
	else
		echo "Found eMMC device:"
		echo "Index.  device  size  model"
		echo "$emmcs"
		index=1
	fi
	local value=$(echo "$emmcs" | grep "^${index}. " | cut -d' ' -f 2)
	export "$var=$value"
}

function main() {
	local bootdisk yn partdev part partdevto to
	get_bootdisk bootdisk
	[ -z "$bootdisk" ] && return 1
	get_emmc $bootdisk to
	[ -n "$to" ] || return 1
	echo -ne "Will install to $to, confirm? y/N [n]\b\b"
	read yn
	if [ "$yn" = Y -o "$yn" = y ]; then
		echo ""
		echo "umount /dev/$to*"
		umount_disk "$to" || return 1
		echo "part /dev/$to"
		part_disk "$to" || return 1
		partprobe "/dev/$to"
		if export_partdevice partdev 1; then
			# umount /boot
			echo "try umount /boot"
			/bin/mount -o noatime,remount,ro /dev/$partdev
			/usr/bin/umount -R -d -l /dev/$partdev 2>/dev/null || /bin/umount -l /dev/$partdev
		fi
		sleep 1
		echo "copy rom data from /dev/$bootdisk to /dev/$to"
		for part in 1 2; do
			# copy boot and rootfs partitions
			export_partdevice partdev $part
			partdevto=${to}p$part
			echo "copy /dev/$partdev to /dev/$partdevto"
			dd if=/dev/$partdev of=/dev/$partdevto bs=1M conv=notrunc
		done
		# mark RESET overlay partition
		partdevto=${to}p3
		echo "mark /dev/$partdevto as RESET"
		echo "RESET000" | dd of="/dev/$partdevto" bs=512 count=1 conv=sync,fsync
		echo "All Done!"
	else
		echo "Bye!"
	fi
}

echo "Copy running OpenWrt to eMMC for amlogic"
echo ""

main
