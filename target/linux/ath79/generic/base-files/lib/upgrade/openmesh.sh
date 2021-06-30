# The U-Boot loader of the OpenMesh devices requires image sizes and
# checksums to be provided in the U-Boot environment.
# The OpenMesh devices come with 2 main partitions - while one is active
# sysupgrade will flash the other. The boot order is changed to boot the
# newly flashed partition. If the new partition can't be booted due to
# upgrade failures the previously used partition is loaded.

cfg_value_get()
{
	local cfg=$1 cfg_opt
	local section=$2 our_section=0
	local param=$3 our_param=

	for cfg_opt in $cfg; do
		[ "$cfg_opt" = "[$section]" ] && our_section=1 && continue
		[ "$our_section" = "1" ] || continue

		our_param=${cfg_opt%%=*}
		[ "$param" = "$our_param" ] && echo ${cfg_opt##*=} && break
	done
}

platform_do_upgrade_openmesh()
{
	local img_path="$1"
	local restore_backup

	local setenv_script="/tmp/fw_env_upgrade"

	local inactive_mtd="$(find_mtd_index $PART_NAME)"
	local inactive_offset="$(cat /sys/class/mtd/mtd${inactive_mtd}/offset)"
	local total_size="$(cat /sys/class/mtd/mtd${inactive_mtd}/size)"
	local total_kbs=$((total_size / 1024))
	local flash_start_mem=0x9f000000
	local data_offset=$((64 * 1024))

	# detect to which flash region the new image is written to.
	#
	# 1. check what is the mtd index for the first flash region on this
	#    device
	# 2. check if the target partition ("inactive") has the mtd index of
	#    the first flash region
	#
	#    - when it is: the new bootseq will be 1,2 and the first region is
	#      modified
	#    - when it isn't: bootseq will be 2,1 and the second region is
	#      modified
	#
	# The detection has to be done via the hardcoded mtd partition because
	# the current boot might be done with the fallback region. Let us
	# assume that the current bootseq is 1,2. The bootloader detected that
	# the image in flash region 1 is corrupt and thus switches to flash
	# region 2. The bootseq in the u-boot-env is now still the same and
	# the sysupgrade code can now only rely on the actual mtd indexes and
	# not the bootseq variable to detect the currently booted flash
	# region/image.
	#
	# In the above example, an implementation which uses bootseq ("1,2") to
	# detect the currently booted image would assume that region 1 is booted
	# and then overwrite the variables for the wrong flash region (aka the
	# one which isn't modified). This could result in a device which doesn't
	# boot anymore to Linux until it was reflashed with ap51-flash.
	local next_boot_part="1"
	local primary_kernel_mtd="3"
	[ "$inactive_mtd" = "$primary_kernel_mtd" ] || next_boot_part="2"

	local cfg_size=$(dd if="$img_path" bs=8 skip=70 count=1 iflag=skip_bytes 2>/dev/null)
	local cfg_length=$((0x$cfg_size))
	local cfg_content=$(dd if="$img_path" bs=$cfg_length skip=$data_offset count=1 iflag=skip_bytes 2>/dev/null)

	local kernel_size=$(dd if="$img_path" bs=8 skip=142 count=1 iflag=skip_bytes 2>/dev/null)
	local kernel_length=$((0x$kernel_size))
	local kernel_kbs=$((kernel_length / 1024))
	local kernel_md5=$(cfg_value_get "$cfg_content" "vmlinux" "md5sum")

	local rootfs_size=$(dd if="$img_path" bs=8 skip=214 count=1 iflag=skip_bytes 2>/dev/null)
	local rootfs_length=$((0x$rootfs_size))
	local rootfs_md5=$(cfg_value_get "$cfg_content" "rootfs" "md5sum")
	local rootfs_checksize=$(cfg_value_get "$cfg_content" "rootfs" "checksize")

	# take care of restoring a saved config
	[ -n "$UPGRADE_BACKUP" ] && restore_backup="${MTD_CONFIG_ARGS} -j ${UPGRADE_BACKUP}"

	# write image parts
	mtd -q erase inactive
	dd if="$img_path" bs=1 skip=$((data_offset + cfg_length + kernel_length)) count=$rootfs_length 2>&- | \
		mtd -n -p $kernel_length $restore_backup write - $PART_NAME
	dd if="$img_path" bs=1024 skip=$((data_offset + cfg_length)) count=$kernel_kbs iflag=skip_bytes 2>&- | \
		mtd -n write - $PART_NAME

	# prepare new u-boot env
	if [ "$next_boot_part" = "1" ]; then
		echo "bootseq 1,2" > $setenv_script
	else
		echo "bootseq 2,1" > $setenv_script
	fi

	printf "kernel_size_%i %i\n" $next_boot_part $kernel_kbs >> $setenv_script
	printf "vmlinux_start_addr 0x%08x\n" $((flash_start_mem + inactive_offset)) >> $setenv_script
	printf "vmlinux_size 0x%08x\n" ${kernel_length} >> $setenv_script
	printf "vmlinux_checksum %s\n" ${kernel_md5} >> $setenv_script

	printf "rootfs_size_%i %i\n" $next_boot_part $((total_kbs - kernel_kbs)) >> $setenv_script
	printf "rootfs_start_addr 0x%08x\n" $((flash_start_mem+inactive_offset+kernel_length)) >> $setenv_script
	printf "rootfs_size %s\n" $rootfs_checksize >> $setenv_script
	printf "rootfs_checksum %s\n" ${rootfs_md5} >> $setenv_script

	# store u-boot env changes
	mkdir -p /var/lock
	fw_setenv -s $setenv_script || {
		echo "failed to update U-Boot environment"
		return 1
	}
}
