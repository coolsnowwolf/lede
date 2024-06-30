# Copyright (C) 2014 OpenWrt.org
#

. /lib/functions.sh

# 'kernel' partition or UBI volume on NAND contains the kernel
CI_KERNPART="${CI_KERNPART:-kernel}"

# 'ubi' partition on NAND contains UBI
# There are also CI_KERN_UBIPART and CI_ROOT_UBIPART if kernel
# and rootfs are on separated UBIs.
CI_UBIPART="${CI_UBIPART:-ubi}"

# 'rootfs' UBI volume on NAND contains the rootfs
CI_ROOTPART="${CI_ROOTPART:-rootfs}"

ubi_mknod() {
	local dir="$1"
	local dev="/dev/$(basename $dir)"

	[ -e "$dev" ] && return 0

	local devid="$(cat $dir/dev)"
	local major="${devid%%:*}"
	local minor="${devid##*:}"
	mknod "$dev" c $major $minor
}

nand_find_volume() {
	local ubidevdir ubivoldir
	ubidevdir="/sys/class/ubi/"
	[ ! -d "$ubidevdir" ] && return 1
	for ubivoldir in $ubidevdir/${1}_*; do
		[ ! -d "$ubivoldir" ] && continue
		if [ "$( cat $ubivoldir/name )" = "$2" ]; then
			basename $ubivoldir
			ubi_mknod "$ubivoldir"
			return 0
		fi
	done
}

nand_find_ubi() {
	local ubidevdir ubidev mtdnum cmtdnum
	mtdnum="$( find_mtd_index $1 )"
	[ ! "$mtdnum" ] && return 1
	for ubidevdir in /sys/class/ubi/ubi*; do
		[ ! -e "$ubidevdir/mtd_num" ] && continue
		cmtdnum="$( cat $ubidevdir/mtd_num )"
		if [ "$mtdnum" = "$cmtdnum" ]; then
			ubidev=$( basename $ubidevdir )
			ubi_mknod "$ubidevdir"
			echo $ubidev
			return 0
		fi
	done
}

nand_get_magic_long() {
	(${3}cat "$1" | dd bs=4 "skip=${2:-0}" count=1 | hexdump -v -n 4 -e '1/1 "%02x"') 2> /dev/null
}

get_magic_long_tar() {
	(tar xO${3}f "$1" "$2" | dd bs=4 count=1 | hexdump -v -n 4 -e '1/1 "%02x"') 2> /dev/null
}

identify() {
	identify_magic_long $(nand_get_magic_long "$@")
}

identify_tar() {
	identify_magic_long $(get_magic_long_tar "$@")
}

identify_if_gzip() {
	if [ "$(identify "$1")" = gzip ]; then echo -n z; fi
}

nand_restore_config() {
	local ubidev=$( nand_find_ubi "${CI_ROOT_UBIPART:-$CI_UBIPART}" )
	local ubivol="$( nand_find_volume $ubidev rootfs_data )"
	if [ ! "$ubivol" ]; then
		ubivol="$( nand_find_volume $ubidev "$CI_ROOTPART" )"
		if [ ! "$ubivol" ]; then
			echo "cannot find ubifs data volume"
			return 1
		fi
	fi
	mkdir /tmp/new_root
	if ! mount -t ubifs /dev/$ubivol /tmp/new_root; then
		echo "cannot mount ubifs volume $ubivol"
		rmdir /tmp/new_root
		return 1
	fi
	if mv "$1" "/tmp/new_root/$BACKUP_FILE"; then
		if umount /tmp/new_root; then
			echo "configuration saved"
			rmdir /tmp/new_root
			return 0
		fi
	else
		umount /tmp/new_root
	fi
	echo "could not save configuration to ubifs volume $ubivol"
	rmdir /tmp/new_root
	return 1
}

nand_remove_ubiblock() {
	local ubivol="$1"

	local ubiblk="ubiblock${ubivol:3}"
	if [ -e "/dev/$ubiblk" ]; then
		umount "/dev/$ubiblk" 2>/dev/null && echo "unmounted /dev/$ubiblk" || :
		if ! ubiblock -r "/dev/$ubivol"; then
			echo "cannot remove $ubiblk"
			return 1
		fi
	fi
}

nand_attach_ubi() {
	local ubipart="$1"
	local has_env="${2:-0}"

	local mtdnum="$( find_mtd_index "$ubipart" )"
	if [ ! "$mtdnum" ]; then
		>&2 echo "cannot find ubi mtd partition $ubipart"
		return 1
	fi

	local ubidev="$( nand_find_ubi "$ubipart" )"
	if [ ! "$ubidev" ]; then
		>&2 ubiattach -m "$mtdnum"
		ubidev="$( nand_find_ubi "$ubipart" )"

		if [ ! "$ubidev" ]; then
			>&2 ubiformat /dev/mtd$mtdnum -y
			>&2 ubiattach -m "$mtdnum"
			ubidev="$( nand_find_ubi "$ubipart" )"

			if [ ! "$ubidev" ]; then
				>&2 echo "cannot attach ubi mtd partition $ubipart"
				return 1
			fi

			if [ "$has_env" -gt 0 ]; then
				>&2 ubimkvol /dev/$ubidev -n 0 -N ubootenv -s 1MiB
				>&2 ubimkvol /dev/$ubidev -n 1 -N ubootenv2 -s 1MiB
			fi
		fi
	fi

	echo "$ubidev"
	return 0
}

nand_detach_ubi() {
	local ubipart="$1"

	local mtdnum="$( find_mtd_index "$ubipart" )"
	if [ ! "$mtdnum" ]; then
		echo "cannot find ubi mtd partition $ubipart"
		return 1
	fi

	local ubidev="$( nand_find_ubi "$ubipart" )"
	if [ "$ubidev" ]; then
		for ubivol in $(find /dev -name "${ubidev}_*" -maxdepth 1 | sort); do
			ubivol="${ubivol:5}"
			nand_remove_ubiblock "$ubivol" || :
			umount "/dev/$ubivol" && echo "unmounted /dev/$ubivol" || :
		done
		if ! ubidetach -m "$mtdnum"; then
			echo "cannot detach ubi mtd partition $ubipart"
			return 1
		fi
	fi
}

nand_upgrade_prepare_ubi() {
	local rootfs_length="$1"
	local rootfs_type="$2"
	local rootfs_data_max="$(fw_printenv -n rootfs_data_max 2> /dev/null)"
	[ -n "$rootfs_data_max" ] && rootfs_data_max=$((rootfs_data_max))

	local kernel_length="$3"
	local has_env="${4:-0}"
	local kern_ubidev
	local root_ubidev

	[ -n "$rootfs_length" -o -n "$kernel_length" ] || return 1

	if [ -n "$CI_KERN_UBIPART" -a -n "$CI_ROOT_UBIPART" ]; then
		kern_ubidev="$( nand_attach_ubi "$CI_KERN_UBIPART" "$has_env" )"
		[ -n "$kern_ubidev" ] || return 1
		root_ubidev="$( nand_attach_ubi "$CI_ROOT_UBIPART" )"
		[ -n "$root_ubidev" ] || return 1
	else
		kern_ubidev="$( nand_attach_ubi "$CI_UBIPART" "$has_env" )"
		[ -n "$kern_ubidev" ] || return 1
		root_ubidev="$kern_ubidev"
	fi

	local kern_ubivol="$( nand_find_volume $kern_ubidev "$CI_KERNPART" )"
	local root_ubivol="$( nand_find_volume $root_ubidev "$CI_ROOTPART" )"
	local data_ubivol="$( nand_find_volume $root_ubidev rootfs_data )"
	[ "$root_ubivol" = "$kern_ubivol" ] && root_ubivol=

	# remove ubiblocks
	[ "$kern_ubivol" ] && { nand_remove_ubiblock $kern_ubivol || return 1; }
	[ "$root_ubivol" ] && { nand_remove_ubiblock $root_ubivol || return 1; }
	[ "$data_ubivol" ] && { nand_remove_ubiblock $data_ubivol || return 1; }

	# kill volumes
	[ "$kern_ubivol" ] && ubirmvol /dev/$kern_ubidev -N "$CI_KERNPART" || :
	[ "$root_ubivol" ] && ubirmvol /dev/$root_ubidev -N "$CI_ROOTPART" || :
	[ "$data_ubivol" ] && ubirmvol /dev/$root_ubidev -N rootfs_data || :

	# create kernel vol
	if [ -n "$kernel_length" ]; then
		if ! ubimkvol /dev/$kern_ubidev -N "$CI_KERNPART" -s $kernel_length; then
			echo "cannot create kernel volume"
			return 1;
		fi
	fi

	# create rootfs vol
	if [ -n "$rootfs_length" ]; then
		local rootfs_size_param
		if [ "$rootfs_type" = "ubifs" ]; then
			rootfs_size_param="-m"
		else
			rootfs_size_param="-s $rootfs_length"
		fi
		if ! ubimkvol /dev/$root_ubidev -N "$CI_ROOTPART" $rootfs_size_param; then
			echo "cannot create rootfs volume"
			return 1;
		fi
	fi

	# create rootfs_data vol for non-ubifs rootfs
	if [ "$rootfs_type" != "ubifs" ]; then
		local rootfs_data_size_param="-m"
		if [ -n "$rootfs_data_max" ]; then
			rootfs_data_size_param="-s $rootfs_data_max"
		fi
		if ! ubimkvol /dev/$root_ubidev -N rootfs_data $rootfs_data_size_param; then
			if ! ubimkvol /dev/$root_ubidev -N rootfs_data -m; then
				echo "cannot initialize rootfs_data volume"
				return 1
			fi
		fi
	fi

	return 0
}

# Write the UBI image to MTD ubi partition
nand_upgrade_ubinized() {
	local ubi_file="$1"
	local gz="$2"

	local ubi_length=$( (${gz}cat "$ubi_file" | wc -c) 2> /dev/null)

	nand_detach_ubi "$CI_UBIPART" || return 1

	local mtdnum="$( find_mtd_index "$CI_UBIPART" )"
	${gz}cat "$ubi_file" | ubiformat "/dev/mtd$mtdnum" -S "$ubi_length" -y -f - && ubiattach -m "$mtdnum"
}

# Write the UBIFS image to UBI rootfs volume
nand_upgrade_ubifs() {
	local ubifs_file="$1"
	local gz="$2"

	local ubifs_length=$( (${gz}cat "$ubifs_file" | wc -c) 2> /dev/null)

	nand_upgrade_prepare_ubi "$ubifs_length" "ubifs" "" "" || return 1

	local ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	local root_ubivol="$(nand_find_volume $ubidev "$CI_ROOTPART")"
	${gz}cat "$ubifs_file" | ubiupdatevol /dev/$root_ubivol -s "$ubifs_length" -
}

# Write the FIT image to UBI kernel volume
nand_upgrade_fit() {
	local fit_file="$1"
	local gz="$2"

	local fit_length=$( (${gz}cat "$fit_file" | wc -c) 2> /dev/null)

	nand_upgrade_prepare_ubi "" "" "$fit_length" "1" || return 1

	local fit_ubidev="$(nand_find_ubi "$CI_UBIPART")"
	local fit_ubivol="$(nand_find_volume $fit_ubidev "$CI_KERNPART")"
	${gz}cat "$fit_file" | ubiupdatevol /dev/$fit_ubivol -s "$fit_length" -
}

# Write images in the TAR file to MTD partitions and/or UBI volumes as required
nand_upgrade_tar() {
	local tar_file="$1"
	local gz="$2"
	local jffs2_markers="${CI_JFFS2_CLEAN_MARKERS:-0}"

	# WARNING: This fails if tar contains more than one 'sysupgrade-*' directory.
	local board_dir="$(tar t${gz}f "$tar_file" | grep -m 1 '^sysupgrade-.*/$')"
	board_dir="${board_dir%/}"

	local kernel_mtd kernel_length
	if [ "$CI_KERNPART" != "none" ]; then
		kernel_mtd="$(find_mtd_index "$CI_KERNPART")"
		kernel_length=$( (tar xO${gz}f "$tar_file" "$board_dir/kernel" | wc -c) 2> /dev/null)
		[ "$kernel_length" = 0 ] && kernel_length=
	fi
	local rootfs_length=$( (tar xO${gz}f "$tar_file" "$board_dir/root" | wc -c) 2> /dev/null)
	[ "$rootfs_length" = 0 ] && rootfs_length=
	local rootfs_type
	[ "$rootfs_length" ] && rootfs_type="$(identify_tar "$tar_file" "$board_dir/root" "$gz")"

	local ubi_kernel_length
	if [ "$kernel_length" ]; then
		if [ "$kernel_mtd" ]; then
			# On some devices, the raw kernel and ubi partitions overlap.
			# These devices brick if the kernel partition is erased.
			# Hence only invalidate kernel for now.
			dd if=/dev/zero bs=4096 count=1 2> /dev/null | \
				mtd write - "$CI_KERNPART"
		else
			ubi_kernel_length="$kernel_length"
		fi
	fi

	local has_env=0
	nand_upgrade_prepare_ubi "$rootfs_length" "$rootfs_type" "$ubi_kernel_length" "$has_env" || return 1

	if [ "$rootfs_length" ]; then
		local ubidev="$( nand_find_ubi "${CI_ROOT_UBIPART:-$CI_UBIPART}" )"
		local root_ubivol="$( nand_find_volume $ubidev "$CI_ROOTPART" )"
		tar xO${gz}f "$tar_file" "$board_dir/root" | \
			ubiupdatevol /dev/$root_ubivol -s "$rootfs_length" -
	fi
	if [ "$kernel_length" ]; then
		if [ "$kernel_mtd" ]; then
			if [ "$jffs2_markers" = 1 ]; then
				flash_erase -j "/dev/mtd${kernel_mtd}" 0 0
				tar xO${gz}f "$tar_file" "$board_dir/kernel" | \
					nandwrite "/dev/mtd${kernel_mtd}" -
			else
				tar xO${gz}f "$tar_file" "$board_dir/kernel" | \
					mtd write - "$CI_KERNPART"
			fi
		else
			local ubidev="$( nand_find_ubi "${CI_KERN_UBIPART:-$CI_UBIPART}" )"
			local kern_ubivol="$( nand_find_volume $ubidev "$CI_KERNPART" )"
			tar xO${gz}f "$tar_file" "$board_dir/kernel" | \
				ubiupdatevol /dev/$kern_ubivol -s "$kernel_length" -
		fi
	fi

	return 0
}

nand_verify_if_gzip_file() {
	local file="$1"
	local gz="$2"

	if [ "$gz" = z ]; then
		echo "verifying compressed sysupgrade file integrity"
		if ! gzip -t "$file"; then
			echo "corrupted compressed sysupgrade file"
			return 1
		fi
	fi
}

nand_verify_tar_file() {
	local file="$1"
	local gz="$2"

	echo "verifying sysupgrade tar file integrity"
	if ! tar xO${gz}f "$file" > /dev/null; then
		echo "corrupted sysupgrade tar file"
		return 1
	fi
}

nand_do_flash_file() {
	local file="$1"

	local gz="$(identify_if_gzip "$file")"
	local file_type="$(identify "$file" "" "$gz")"

	[ ! "$(find_mtd_index "$CI_UBIPART")" ] && CI_UBIPART=rootfs

	case "$file_type" in
		"fit")
			nand_verify_if_gzip_file "$file" "$gz" || return 1
			nand_upgrade_fit "$file" "$gz"
			;;
		"ubi")
			nand_verify_if_gzip_file "$file" "$gz" || return 1
			nand_upgrade_ubinized "$file" "$gz"
			;;
		"ubifs")
			nand_verify_if_gzip_file "$file" "$gz" || return 1
			nand_upgrade_ubifs "$file" "$gz"
			;;
		*)
			nand_verify_tar_file "$file" "$gz" || return 1
			nand_upgrade_tar "$file" "$gz"
			;;
	esac
}

nand_do_restore_config() {
	local conf_tar="/tmp/sysupgrade.tgz"
	[ ! -f "$conf_tar" ] || nand_restore_config "$conf_tar"
}

# Recognize type of passed file and start the upgrade process
nand_do_upgrade() {
	local file="$1"

	sync
	nand_do_flash_file "$file" && nand_do_upgrade_success
	nand_do_upgrade_failed
}

nand_do_upgrade_success() {
	if nand_do_restore_config && sync; then
		echo "sysupgrade successful"
		umount -a
		reboot -f
	fi
	nand_do_upgrade_failed
}

nand_do_upgrade_failed() {
	sync
	echo "sysupgrade failed"
	# Should we reboot or bring up some failsafe mode instead?
	umount -a
	reboot -f
}

# Check if passed file is a valid one for NAND sysupgrade.
# Currently it accepts 4 types of files:
# 1) UBI: a ubinized image containing required UBI volumes.
# 2) UBIFS: a UBIFS rootfs volume image.
# 3) FIT: a FIT image containing kernel and rootfs.
# 4) TAR: an archive that includes directory "sysupgrade-${BOARD_NAME}" containing
#         a non-empty "CONTROL" file and required partition and/or volume images.
#
# You usually want to call this function in platform_check_image.
#
# $(1): board name, used in case of passing TAR file
# $(2): file to be checked
nand_do_platform_check() {
	local board_name="$1"
	local file="$2"

	local gz="$(identify_if_gzip "$file")"
	local file_type="$(identify "$file" "" "$gz")"
	local control_length=$( (tar xO${gz}f "$file" "sysupgrade-${board_name//,/_}/CONTROL" | wc -c) 2> /dev/null)

	if [ "$control_length" = 0 ]; then
		control_length=$( (tar xO${gz}f "$file" "sysupgrade-${board_name//_/,}/CONTROL" | wc -c) 2> /dev/null)
	fi

	if [ "$control_length" != 0 ]; then
		nand_verify_tar_file "$file" "$gz" || return 1
	else
		nand_verify_if_gzip_file "$file" "$gz" || return 1
		if [ "$file_type" != "fit" -a "$file_type" != "ubi" -a "$file_type" != "ubifs" ]; then
			echo "invalid sysupgrade file"
			return 1
		fi
	fi

	return 0
}
