# ======== dev note ========
# for following buffalo MT7622 devices:
#
# - WSR-2533DHP2 (trx magic: "DHP2")
# - WSR-2533DHP3 (trx magic: "DHP3")
# - WSR-3200AX4S (trx magic: "DHP3")
#
# sysupgrade-tar image:
#
# This is for normal upgrading for OpenWrt.
# use nand_do_upgrade with CI_KERNPART="firmware"
#
# - if the size of new kernel is not equal with the current kernel's
#   -> block upgrade and print a message about using TRX + UBI
#      formatted image
#   (should be flashed the new ubi contains rootfs + rootfs_data
#   with the offset (=new padded kernel's end) if this case? But
#   it maybe too hard for writing scripts...)
#
# TRX + UBI formatted image:
#
# This is for upgrading if the new kernel is larger than the
# current kernel.
#
# ex:
#   - stock firmware is installed in the flash and booted with
#     OpenWrt initramfs image
#   - kernel partition is increased from 4MiB in OpenWrt in the
#     future
#
# packing TRX + UBI formatted image by tar is needed for image validation
# with the metadata in the future?
# ====== dev note end ======
#
# The mtd partitions "firmware" and "Kernel2" on NAND flash are os-image
# partitions. These partitions are called as "Image1/Image2" in U-Boot
# on WSR-2533DHP2, and they are checked conditions when booting.
# "Image1" is always used for booting.
#
# == U-Boot Behaviors ==
# - "Image1"/"Image2" images are good, images are different or
#   "Image2" image is broken
#   -> writes os-image to "Image2" from "Image1"
#
# - "Image1" image is broken
#   -> writes os-image to "Image1" from "Image2"
#
# - "Image1"/"Image2" images are broken
#   -> fall to U-Boot command line

buffalo_check_image() {
	local board="$1"
	local boardname="$(echo $board | tr ',' '_')"
	local magic="$2"
	local fw_image="$3"

	# return error state if TRX + UBI formatted image specified
	# to notify about configurations
	if [ "$magic" = "44485032" -o "$magic" = "44485033" ]; then
		echo "Your configurations won't be saved if factory-uboot.bin image specified."
		echo "But if you want to upgrade, please execute sysupgrade with \"-F\" option."
		return 1
	fi

	# check if valid tar file specifed
	if ! tar tf "$fw_image" &>/dev/null; then
		echo "Specified file is not a tar archive: $fw_image"
		return 1
	fi

	local control_len=$( (tar xf $fw_image sysupgrade-$boardname/CONTROL -O | wc -c) 2> /dev/null)

	# check if valid sysupgrade tar archive
	if [ "$control_len" = "0" ]; then
		echo "Invalid sysupgrade file: $fw_image"
		return 1
	fi

	local kern_part_len=$(grep "\"linux\"" /proc/mtd | sed "s/mtd[0-9]*:[ \t]*\([^ \t]*\).*/\1/")
	[ -z "$kern_part_len" ] && {
		echo "Unable to get \"linux\" partition size"
		return 1
	}
	kern_part_len=$((0x$kern_part_len))

	# this also checks if the sysupgrade image is for correct models
	local kern_bin_len=$( (tar xf $fw_image sysupgrade-${boardname}/kernel -O | wc -c) 2> /dev/null)
	if [ -z "$kern_bin_len" ]; then
		echo "Failed to get new kernel size, is valid sysupgrade image specified for the device?"
		return 1
	fi

	# kernel binary has a trx header (len: 28 (0x1c))
	kern_bin_len=$((kern_bin_len - 28))

	if [ "$kern_bin_len" != "$kern_part_len" ]; then
		echo -n "The length of new kernel is invalid for current "
		echo "\"linux\" partition, please use factory-uboot.bin image."
		echo "\"linux\" partition: $kern_part_len, new kernel: $kern_bin_len"
		return 1
	fi
}

# for TRX + UBI formatted image
buffalo_upgrade_ubinized() {
	sync
	echo 3 > /proc/sys/vm/drop_caches

	local mtdnum="$( find_mtd_index "ubi" )"
	# if no "ubi", don't return error for the purpose of recovery
	# ex: recovery after accidental erasing "firmware" partition
	if [ ! "$mtdnum" ]; then
		echo "cannot find ubi mtd partition \"ubi\", skip detachment"
	else
		ubidetach -m "$mtdnum"
	fi

	# erase all data in "firmware"
	mtd erase "${PART_NAME}"
	# write TRX + UBI formatted image to "firmware"
	get_image "$1" | mtd $MTD_ARGS write - "${PART_NAME:-firmware}"
	if [ $? -ne 0 ]; then
		echo "Failed to write the specified image."
		exit 1
	fi
}
