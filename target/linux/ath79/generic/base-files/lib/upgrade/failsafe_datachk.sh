# U-Boot with the datachk patchset requires image sizes, offsets,
# and checksums to be provided in the U-Boot environment.
# This script is based on the dualboot version for devices that come with 2 OS partitions.
# For Senao boards with a "failsafe" partition image, the process is almost the same.
# Instead of booting a secondary instalation on checksum failure,
# the failsafe image is booted instead.
# These boards also use the OKLI lzma kernel loader and mtd-concat
# So the kernel check is for the loader, the rootfs check is for kernel + rootfs

platform_do_upgrade_failsafe_datachk() {
	local setenv_script="/tmp/fw_env_upgrade"

	local flash_base=0x9f000000

	local kernel_mtd=$(find_mtd_index ${KERNEL_PART:-kernel})
	local rootfs_mtd=$(find_mtd_index ${ROOTFS_PART:-rootfs})

	local kernel_offset=$(cat /sys/class/mtd/mtd${kernel_mtd}/offset)
	local rootfs_offset=$(cat /sys/class/mtd/mtd${rootfs_mtd}/offset)

	if [ -n "$IMAGE_LIST" ]; then
		KERNEL_FILE=$($IMAGE_LIST | grep $KERNEL_FILE)
		ROOTFS_FILE=$($IMAGE_LIST | grep $ROOTFS_FILE)
	fi

	local kernel_size=$($IMAGE_CMD $KERNEL_FILE | wc -c)
	local rootfs_size=$($IMAGE_CMD $ROOTFS_FILE | wc -c)

	# rootfs without JFFS2
	local rootfs_blocks=$((rootfs_size / 4096))
	rootfs_size=$((rootfs_blocks * 4096))

	local kernel_md5=$($IMAGE_CMD $KERNEL_FILE | md5sum | cut -d ' ' -f1)
	local rootfs_md5=$($IMAGE_CMD $ROOTFS_FILE | dd bs=4k count=$rootfs_blocks iflag=fullblock | md5sum | cut -d ' ' -f1)

	# prepare new u-boot-env vars
	printf "vmlinux_start_addr 0x%08x\n" $((flash_base + kernel_offset)) >> $setenv_script
	printf "vmlinux_size 0x%08x\n" ${kernel_size} >> $setenv_script
	printf "vmlinux_checksum %s\n" ${kernel_md5} >> $setenv_script

	printf "rootfs_start_addr 0x%08x\n" $((flash_base + rootfs_offset)) >> $setenv_script
	printf "rootfs_size 0x%08x\n" ${rootfs_size} >> $setenv_script
	printf "rootfs_checksum %s\n" ${rootfs_md5} >> $setenv_script

	# store u-boot-env
	mkdir -p /var/lock
	fw_setenv -s $setenv_script || {
		echo 'failed to update U-Boot environment'
		exit 1
	}

	# sysupgrade
	sleep 2
	sync
	echo 3 > /proc/sys/vm/drop_caches
	$IMAGE_CMD $KERNEL_FILE | mtd $MTD_ARGS write - ${KERNEL_PART:-kernel}
	sleep 2
	sync
	if [ -n "$UPGRADE_BACKUP" ]; then
		$IMAGE_CMD $ROOTFS_FILE | mtd $MTD_ARGS $MTD_CONFIG_ARGS -j $UPGRADE_BACKUP write - ${ROOTFS_PART:-rootfs}
	else
		$IMAGE_CMD $ROOTFS_FILE | mtd $MTD_ARGS write - ${ROOTFS_PART:-rootfs}
	fi
}
