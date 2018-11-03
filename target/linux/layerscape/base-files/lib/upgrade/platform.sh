#!/bin/sh
#
# Copyright 2015-2018 Traverse Technologies
#
platform_do_upgrade_traverse_nandubi() {
	bootsys=$(fw_printenv bootsys | awk -F= '{{print $2}}')
	newbootsys=2
	if [ "$bootsys" -eq "2" ]; then
		newbootsys=1
	fi
	mkdir -p /tmp/image
	cd /tmp/image
	get_image "$1" > image.tar
	ls -la image.tar
	files=$(tar -tf image.tar)
	echo "Files in image:"
	echo $files
	for f in $files
	do
		part_name=$(echo $f | awk -F '/' '{{print $2}}')
		if [ -z "$part_name" ] || [ "$part_name" = "CONTROL" ]; then
			continue
		fi

		[ "$part_name" = "root" ] && part_name="rootfs"

		volume=$part_name
		if [ "$part_name" = "kernel" ] || [ "$part_name" = "rootfs" ]; then
			volume="${part_name}${newbootsys}"
		fi
		volume_id=$(ubinfo -d 0 --name $volume | awk '/Volume ID/ {print $3}')
		file_size=$(tar -tvf image.tar $f | awk '{{print $3}}')
		echo "$f size $file_size"
		tar -xOf image.tar $f | ubiupdatevol -s $file_size /dev/ubi0_$volume_id -
		
		echo "$volume upgraded"
	done
	fw_setenv bootsys $newbootsys
	echo "Upgrade complete"
}
platform_copy_config() {
	bootsys=$(fw_printenv bootsys | awk -F= '{{print $2}}')
	rootvol=rootfs$bootsys
	volume_id=$(ubinfo -d 0 --name $rootvol | awk '/Volume ID/ {print $3}')
	mkdir -p /mnt/oldsys
	mount -t ubifs -o rw,noatime /dev/ubi0_$volume_id /mnt/oldsys
	cp -af "$CONF_TAR" /mnt/oldsys
	umount /mnt/oldsys
}
platform_check_image() {
	local board=$(board_name)

	case "$board" in
	traverse,ls1043v | \
	traverse,ls1043s | \
	traverse,five64)
		local tar_file="$1"
		local kernel_length=$( (tar xf $tar_file sysupgrade-traverse-five64/kernel -O | wc -c) 2> /dev/null)
		local rootfs_length=$( (tar xf $tar_file sysupgrade-traverse-five64/root -O | wc -c) 2> /dev/null)
		[ "$kernel_length" -eq 0 -o "$rootfs_length" -eq 0 ] && {
			echo "The upgrade image is corrupt."
			return 1
		}
		return 0
		;;
	*)
		echo "Sysupgrade is not currently supported on $board"
		;;
	esac

	return 1
}
platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	traverse,ls1043v | \
	traverse,ls1043s | \
	traverse,five64)
		platform_do_upgrade_traverse_nandubi "$ARGV"
		;;
	*)
		echo "Sysupgrade is not currently supported on $board"
		;;
	esac
}
platform_pre_upgrade() {
	# Force the creation of fw_printenv.lock
	mkdir -p /var/lock
	touch /var/lock/fw_printenv.lock
	
	export RAMFS_COPY_BIN="/usr/sbin/fw_printenv /usr/sbin/fw_setenv /usr/sbin/ubinfo /bin/echo ${RAMFS_COPY_BIN}"
	export RAMFS_COPY_DATA="/etc/fw_env.config /var/lock/fw_printenv.lock ${RAMFS_COPY_DATA}"
}
