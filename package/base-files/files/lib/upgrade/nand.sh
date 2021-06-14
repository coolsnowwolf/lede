# Copyright (C) 2014 OpenWrt.org
#

. /lib/functions.sh

# 'kernel' partition or UBI volume on NAND contains the kernel
CI_KERNPART="${CI_KERNPART:-kernel}"

# 'ubi' partition on NAND contains UBI
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
	ubidevdir="/sys/devices/virtual/ubi/$1"
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
	local ubidevdir ubidev mtdnum
	mtdnum="$( find_mtd_index $1 )"
	[ ! "$mtdnum" ] && return 1
	for ubidevdir in /sys/devices/virtual/ubi/ubi*; do
		[ ! -d "$ubidevdir" ] && continue
		cmtdnum="$( cat $ubidevdir/mtd_num )"
		[ ! "$mtdnum" ] && continue
		if [ "$mtdnum" = "$cmtdnum" ]; then
			ubidev=$( basename $ubidevdir )
			ubi_mknod "$ubidevdir"
			echo $ubidev
			return 0
		fi
	done
}

nand_get_magic_long() {
	dd if="$1" skip=$2 bs=4 count=1 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'
}

get_magic_long_tar() {
	( tar xf $1 $2 -O | dd bs=4 count=1 | hexdump -v -n 4 -e '1/1 "%02x"') 2> /dev/null
}

identify_magic() {
	local magic=$1
	case "$magic" in
		"55424923")
			echo "ubi"
			;;
		"31181006")
			echo "ubifs"
			;;
		"68737173")
			echo "squashfs"
			;;
		"d00dfeed")
			echo "fit"
			;;
		"4349"*)
			echo "combined"
			;;
		*)
			echo "unknown $magic"
			;;
	esac
}


identify() {
	identify_magic $(nand_get_magic_long "$1" "${2:-0}")
}

identify_tar() {
	identify_magic $(get_magic_long_tar "$1" "$2")
}

nand_restore_config() {
	sync
	local ubidev=$( nand_find_ubi $CI_UBIPART )
	local ubivol="$( nand_find_volume $ubidev rootfs_data )"
	[ ! "$ubivol" ] &&
		ubivol="$( nand_find_volume $ubidev $CI_ROOTPART )"
	mkdir /tmp/new_root
	if ! mount -t ubifs /dev/$ubivol /tmp/new_root; then
		echo "mounting ubifs $ubivol failed"
		rmdir /tmp/new_root
		return 1
	fi
	mv "$1" "/tmp/new_root/sysupgrade.tgz"
	umount /tmp/new_root
	sync
	rmdir /tmp/new_root
}

nand_upgrade_prepare_ubi() {
	local rootfs_length="$1"
	local rootfs_type="$2"
	local kernel_length="$3"
	local has_env="${4:-0}"
	local rootfs_data_max="$(fw_printenv -n rootfs_data_max 2>/dev/null)"
	[ -n "$rootfs_data_max" ] && rootfs_data_max=$(($rootfs_data_max))

	[ -n "$rootfs_length" -o -n "$kernel_length" ] || return 1

	local mtdnum="$( find_mtd_index "$CI_UBIPART" )"
	if [ ! "$mtdnum" ]; then
		echo "cannot find ubi mtd partition $CI_UBIPART"
		return 1
	fi

	local ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	if [ ! "$ubidev" ]; then
		ubiattach -m "$mtdnum"
		sync
		ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	fi

	if [ ! "$ubidev" ]; then
		ubiformat /dev/mtd$mtdnum -y
		ubiattach -m "$mtdnum"
		sync
		ubidev="$( nand_find_ubi "$CI_UBIPART" )"
		[ "$has_env" -gt 0 ] && {
			ubimkvol /dev/$ubidev -n 0 -N ubootenv -s 1MiB
			ubimkvol /dev/$ubidev -n 1 -N ubootenv2 -s 1MiB
		}
	fi

	local kern_ubivol="$( nand_find_volume $ubidev $CI_KERNPART )"
	local root_ubivol="$( nand_find_volume $ubidev $CI_ROOTPART )"
	local data_ubivol="$( nand_find_volume $ubidev rootfs_data )"

	local ubiblk ubiblkvol
	for ubiblk in /dev/ubiblock*_? ; do
		[ -e "$ubiblk" ] || continue
		echo "removing ubiblock${ubiblk:13}"
		ubiblkvol=ubi${ubiblk:13}
		if ! ubiblock -r /dev/$ubiblkvol; then
			echo "cannot remove $ubiblk"
			return 1
		fi
	done

	# kill volumes
	[ "$kern_ubivol" ] && ubirmvol /dev/$ubidev -N $CI_KERNPART || true
	[ "$root_ubivol" -a "$root_ubivol" != "$kern_ubivol" ] && ubirmvol /dev/$ubidev -N $CI_ROOTPART || true
	[ "$data_ubivol" ] && ubirmvol /dev/$ubidev -N rootfs_data || true

	# update kernel
	if [ -n "$kernel_length" ]; then
		if ! ubimkvol /dev/$ubidev -N $CI_KERNPART -s $kernel_length; then
			echo "cannot create kernel volume"
			return 1;
		fi
	fi

	# update rootfs
	if [ -n "$rootfs_length" ]; then
		local rootfs_size_param
		if [ "$rootfs_type" = "ubifs" ]; then
			rootfs_size_param="-m"
		else
			rootfs_size_param="-s $rootfs_length"
		fi
		if ! ubimkvol /dev/$ubidev -N $CI_ROOTPART $rootfs_size_param; then
			echo "cannot create rootfs volume"
			return 1;
		fi
	fi

	# create rootfs_data for non-ubifs rootfs
	if [ "$rootfs_type" != "ubifs" ]; then
		local availeb=$(cat /sys/devices/virtual/ubi/$ubidev/avail_eraseblocks)
		local ebsize=$(cat /sys/devices/virtual/ubi/$ubidev/eraseblock_size)
		local avail_size=$(( $availeb * $ebsize ))
		local rootfs_data_size_param="-m"
		if [ -n "$rootfs_data_max" ] &&
		   [ "$rootfs_data_max" != "0" ] &&
		   [ "$rootfs_data_max" -le "$avail_size" ]; then
			rootfs_data_size_param="-s $rootfs_data_max"
		fi
		if ! ubimkvol /dev/$ubidev -N rootfs_data $rootfs_data_size_param; then
			echo "cannot initialize rootfs_data volume"
			return 1
		fi
	fi
	sync
	return 0
}

nand_do_upgrade_success() {
	local conf_tar="/tmp/sysupgrade.tgz"

	sync
	[ -f "$conf_tar" ] && nand_restore_config "$conf_tar"
	echo "sysupgrade successful"
	umount -a
	reboot -f
}

# Flash the UBI image to MTD partition
nand_upgrade_ubinized() {
	local ubi_file="$1"
	local mtdnum="$(find_mtd_index "$CI_UBIPART")"

	[ ! "$mtdnum" ] && {
		CI_UBIPART="rootfs"
		mtdnum="$(find_mtd_index "$CI_UBIPART")"
	}

	if [ ! "$mtdnum" ]; then
		echo "cannot find mtd device $CI_UBIPART"
		umount -a
		reboot -f
	fi

	local mtddev="/dev/mtd${mtdnum}"
	ubidetach -p "${mtddev}" || true
	sync
	ubiformat "${mtddev}" -y -f "${ubi_file}"
	ubiattach -p "${mtddev}"
	nand_do_upgrade_success
}

# Write the UBIFS image to UBI volume
nand_upgrade_ubifs() {
	local rootfs_length=$( (cat $1 | wc -c) 2> /dev/null)

	nand_upgrade_prepare_ubi "$rootfs_length" "ubifs" "" ""

	local ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	local root_ubivol="$(nand_find_volume $ubidev $CI_ROOTPART)"
	ubiupdatevol /dev/$root_ubivol -s $rootfs_length $1

	nand_do_upgrade_success
}

nand_upgrade_fit() {
	local fit_file="$1"
	local fit_length="$(wc -c < "$fit_file")"

	nand_upgrade_prepare_ubi "" "" "$fit_length" "1"

	local fit_ubidev="$(nand_find_ubi "$CI_UBIPART")"
	local fit_ubivol="$(nand_find_volume $fit_ubidev "$CI_KERNPART")"
	ubiupdatevol /dev/$fit_ubivol -s $fit_length $fit_file

	nand_do_upgrade_success
}

nand_upgrade_tar() {
	local tar_file="$1"
	local kernel_mtd="$(find_mtd_index $CI_KERNPART)"

	local board_dir=$(tar tf "$tar_file" | grep -m 1 '^sysupgrade-.*/$')
	board_dir=${board_dir%/}

	kernel_length=$( (tar xf "$tar_file" ${board_dir}/kernel -O | wc -c) 2> /dev/null)
	local has_rootfs=0
	local rootfs_length
	local rootfs_type

	tar tf "$tar_file" ${board_dir}/root 1>/dev/null 2>/dev/null && has_rootfs=1
	[ "$has_rootfs" = "1" ] && {
		rootfs_length=$( (tar xf "$tar_file" ${board_dir}/root -O | wc -c) 2> /dev/null)
		rootfs_type="$(identify_tar "$tar_file" ${board_dir}/root)"
	}

	local has_kernel=1
	local has_env=0

	[ "$kernel_length" != 0 -a -n "$kernel_mtd" ] && {
		tar xf "$tar_file" ${board_dir}/kernel -O | mtd write - $CI_KERNPART
	}
	[ "$kernel_length" = 0 -o ! -z "$kernel_mtd" ] && has_kernel=

	nand_upgrade_prepare_ubi "$rootfs_length" "$rootfs_type" "${has_kernel:+$kernel_length}" "$has_env"

	local ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	[ "$has_kernel" = "1" ] && {
		local kern_ubivol="$( nand_find_volume $ubidev $CI_KERNPART )"
		tar xf "$tar_file" ${board_dir}/kernel -O | \
			ubiupdatevol /dev/$kern_ubivol -s $kernel_length -
	}

	[ "$has_rootfs" = "1" ] && {
		local root_ubivol="$( nand_find_volume $ubidev $CI_ROOTPART )"
		tar xf "$tar_file" ${board_dir}/root -O | \
			ubiupdatevol /dev/$root_ubivol -s $rootfs_length -
	}
	nand_do_upgrade_success
}

# Recognize type of passed file and start the upgrade process
nand_do_upgrade() {
	if [ -n "$IS_PRE_UPGRADE" ]; then
		# Previously, nand_do_upgrade was called from the platform_pre_upgrade
		# hook; this piece of code handles scripts that haven't been
		# updated. All scripts should gradually move to call nand_do_upgrade
		# from platform_do_upgrade instead.
		export do_upgrade="nand_do_upgrade '$1'"
		return
	fi

	local file_type=$(identify $1)

	if type 'platform_nand_pre_upgrade' >/dev/null 2>/dev/null; then
		platform_nand_pre_upgrade "$1"
	fi

	[ ! "$(find_mtd_index "$CI_UBIPART")" ] && CI_UBIPART="rootfs"

	case "$file_type" in
		"fit")		nand_upgrade_fit $1;;
		"ubi")		nand_upgrade_ubinized $1;;
		"ubifs")	nand_upgrade_ubifs $1;;
		*)		nand_upgrade_tar $1;;
	esac
}

# Check if passed file is a valid one for NAND sysupgrade. Currently it accepts
# 3 types of files:
# 1) UBI - should contain an ubinized image, header is checked for the proper
#    MAGIC
# 2) UBIFS - should contain UBIFS partition that will replace "rootfs" volume,
#    header is checked for the proper MAGIC
# 3) TAR - archive has to include "sysupgrade-BOARD" directory with a non-empty
#    "CONTROL" file (at this point its content isn't verified)
#
# You usually want to call this function in platform_check_image.
#
# $(1): board name, used in case of passing TAR file
# $(2): file to be checked
nand_do_platform_check() {
	local board_name="$1"
	local tar_file="$2"
	local control_length=$( (tar xf $tar_file sysupgrade-$board_name/CONTROL -O | wc -c) 2> /dev/null)
	local file_type="$(identify $2)"

	[ "$control_length" = 0 -a "$file_type" != "ubi" -a "$file_type" != "ubifs" -a "$file_type" != "fit" ] && {
		echo "Invalid sysupgrade file."
		return 1
	}

	return 0
}
