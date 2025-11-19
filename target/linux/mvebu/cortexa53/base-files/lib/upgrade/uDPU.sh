udpu_check_emmc() {
# uDPU uses combined ext4 and f2fs partitions.
# partition layout:
#	1. boot	(ext4)
#	2. recovery  (ext4)
#	3. rootfs (f2fs)
#	4. misc (f2fs)

	# Check which device is available, depending on the board revision
	if [ -b "/dev/mmcblk1" ]; then
		emmc_dev=/dev/mmcblk1
	elif [ -b "/dev/mmcblk0" ]; then
		emmc_dev=/dev/mmcblk0
	else
		echo "Cannot detect eMMC flash, aborting.."
		exit 1
	fi
}

udpu_part_prep() {
	 if grep -q "$1" /proc/mounts; then
		mounted_part="$(grep -m 1 $1 /proc/mounts | awk '{print $2}')"
		umount "$mounted_part"
		grep -woq "$mounted_part" /proc/mounts && umount -l "$mounted_part"
	fi
}

udpu_do_part_check() {
	local emmc_parts="1 2 3 4"
	local part_valid="1"

	# Check if the block devices exist
	for num in ${emmc_parts}; do
		[ ! -b ${emmc_dev}p${num} ] && part_valid="0"
	done

	# If partitions are missing create a new partition table
	if [ "$part_valid" != "1" ]; then
		printf "Invalid partition table, creating a new one\n"
		printf "o\nn\np\n1\n\n+256M\nn\np\n2\n\n+256M\nn\np\n3\n\n+1536M\nn\np\n\n\nw\n" | fdisk -W always $emmc_dev  > /dev/null 2>&1

		# Format the /misc part right away as we will need it for the firmware
		printf "Formating /misc partition, this make take a while..\n"
		udpu_part_prep ${emmc_dev}p4
		if mkfs.f2fs -q -l misc -O extra_attr,compression ${emmc_dev}p4; then
			printf "/misc partition formated successfully\n"
		else
			printf "/misc partition formatting failed\n"
		fi

		udpu_do_initial_setup
	else
		printf "Partition table looks ok\n"
	fi
}

udpu_do_misc_prep() {
	if ! grep -woq /misc /proc/mounts; then
		mkdir -p /misc

		# If the mount fails, try to reformat partition
		# Leaving possiblity for multiple iterations
		if ! mount ${emmc_dev}p4 /misc; then
			printf "Error while mounting /misc, trying to reformat..\n"

			format_count=0
			while [ "$format_count" -lt "1" ]; do
				udpu_part_prep ${emmc_dev}p4
				mkfs.f2fs -q -l misc -O extra_attr,compression ${emmc_dev}p4
				if ! mount ${emmc_dev}p4 /misc; then
					umount -l /misc
					printf "Failed while mounting /misc\n"
					format_count=$((format_count +1))
				else
					printf "Mounted /misc successfully\n"
					break
				fi
			done
		fi
	fi
}

udpu_do_initial_setup() {
	# Prepare /recovery parition
	udpu_part_prep ${emmc_dev}p2
	mkfs.ext4 -qF ${emmc_dev}p2 2>&1 /dev/null

	# Prepare /boot partition
	udpu_part_prep ${emmc_dev}p1
	mkfs.ext4 -qF ${emmc_dev}p1 2>&1 /dev/null

	# Prepare /root partition
	printf "Formating /root partition, this may take a while..\n"
	udpu_part_prep ${emmc_dev}p3
	mkfs.f2fs -q -l rootfs -O extra_attr,compression ${emmc_dev}p3 && printf "/root partition reformated\n"
}

udpu_do_regular_upgrade() {
	# Clean /boot partition - mfks.ext4 is not available in chroot
	grep -woq /boot /proc/mounts && umount /boot
	mkdir -p /tmp/boot
	mount ${emmc_dev}p1 /tmp/boot
	rm -rf /tmp/boot/*

	# Clean /root partition - mkfs.f2fs is not available in chroot
	grep -woq /dev/root /proc/mounts && umount /
	mkdir -p /tmp/rootpart
	mount ${emmc_dev}p3 /tmp/rootpart
	rm -rf /tmp/rootpart/*
}

platform_do_upgrade_uDPU() {
	udpu_check_emmc

	# Prepare and extract firmware on /misc partition
	udpu_do_misc_prep

	[ -f "/misc/firmware" ] && rm -r /misc/firmware
	mkdir -p /misc/firmware
	tar xzf "$1" -C /misc/firmware/

	udpu_do_regular_upgrade

	printf "Updating /boot partition\n"
	if tar xzf /misc/firmware/boot.tgz -C /tmp/boot; then
		printf "/boot partition updated successfully\n"
	else
		printf "/boot partition update failed\n"
	fi
	sync

	printf "Updating /root partition\n"
	if tar xzf /misc/firmware/rootfs.tgz -C /tmp/rootpart; then
		printf "/root partition updated successfully\n"
	else
		printf "/root partition update failed\n"
	fi
	sync

	# Saving configuration files over sysupgrade
	platform_copy_config_uDPU

	# Remove tmp mounts
	tmp_parts=$(grep "${emmc_dev}" /proc/mounts | awk '{print $2}')
	for part in ${tmp_parts}; do
		umount "$part"
		# Force umount is necessary
		grep -q "${part}" /proc/mounts && umount -l "$part"
	done

	# Sysupgrade complains about /tmp and /dev, so we can detach them here
	umount -l /tmp
	umount -l /dev
}

platform_copy_config_uDPU() {
	# Config is saved on the /misc partition and copied on the rootfs after the reboot
	if [ -f "$UPGRADE_BACKUP" ]; then
		cp -f "$UPGRADE_BACKUP" "/misc/$BACKUP_FILE"
		sync
	fi
}
