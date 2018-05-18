#
# Copyright (C) 2011 OpenWrt.org
#

. /lib/functions/system.sh

PART_NAME=firmware
RAMFS_COPY_BIN='nandwrite'

CI_BLKSZ=65536
CI_LDADR=0x80060000

PLATFORM_DO_UPGRADE_COMBINED_SEPARATE_MTD=0

platform_find_partitions() {
	local first dev size erasesize name
	while read dev size erasesize name; do
		name=${name#'"'}; name=${name%'"'}
		case "$name" in
			vmlinux.bin.l7|vmlinux|kernel|linux|linux.bin|rootfs|filesystem)
				if [ -z "$first" ]; then
					first="$name"
				else
					echo "$erasesize:$first:$name"
					break
				fi
			;;
		esac
	done < /proc/mtd
}

platform_find_kernelpart() {
	local part
	for part in "${1%:*}" "${1#*:}"; do
		case "$part" in
			vmlinux.bin.l7|vmlinux|kernel|linux|linux.bin)
				echo "$part"
				break
			;;
		esac
	done
}

platform_find_rootfspart() {
	local part
	for part in "${1%:*}" "${1#*:}"; do
		[ "$part" != "$2" ] && echo "$part" && break
	done
}

platform_do_upgrade_combined() {
	local partitions=$(platform_find_partitions)
	local kernelpart=$(platform_find_kernelpart "${partitions#*:}")
	local erase_size=$((0x${partitions%%:*})); partitions="${partitions#*:}"
	local kern_length=0x$(dd if="$1" bs=2 skip=1 count=4 2>/dev/null)
	local kern_blocks=$(($kern_length / $CI_BLKSZ))
	local root_blocks=$((0x$(dd if="$1" bs=2 skip=5 count=4 2>/dev/null) / $CI_BLKSZ))

	if [ -n "$partitions" ] && [ -n "$kernelpart" ] && \
	   [ ${kern_blocks:-0} -gt 0 ] && \
	   [ ${root_blocks:-0} -gt 0 ] && \
	   [ ${erase_size:-0} -gt 0 ];
	then
		local rootfspart=$(platform_find_rootfspart "$partitions" "$kernelpart")
		local append=""
		[ -f "$CONF_TAR" -a "$SAVE_CONFIG" -eq 1 ] && append="-j $CONF_TAR"

		if [ "$PLATFORM_DO_UPGRADE_COMBINED_SEPARATE_MTD" -ne 1 ]; then
		    ( dd if="$1" bs=$CI_BLKSZ skip=1 count=$kern_blocks 2>/dev/null; \
		      dd if="$1" bs=$CI_BLKSZ skip=$((1+$kern_blocks)) count=$root_blocks 2>/dev/null ) | \
			    mtd -r $append -F$kernelpart:$kern_length:$CI_LDADR,rootfs write - $partitions
		elif [ -n "$rootfspart" ]; then
		    dd if="$1" bs=$CI_BLKSZ skip=1 count=$kern_blocks 2>/dev/null | \
			    mtd write - $kernelpart
		    dd if="$1" bs=$CI_BLKSZ skip=$((1+$kern_blocks)) count=$root_blocks 2>/dev/null | \
			    mtd -r $append write - $rootfspart
		fi
	fi
	PLATFORM_DO_UPGRADE_COMBINED_SEPARATE_MTD=0
}

tplink_get_image_hwid() {
	get_image "$@" | dd bs=4 count=1 skip=16 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'
}

tplink_get_image_mid() {
	get_image "$@" | dd bs=4 count=1 skip=17 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'
}

tplink_get_image_boot_size() {
	get_image "$@" | dd bs=4 count=1 skip=37 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'
}

tplink_pharos_check_image() {
	local magic_long="$(get_magic_long "$1")"
	[ "$magic_long" != "7f454c46" ] && {
		echo "Invalid image magic '$magic_long'"
		return 1
	}

	local model_string="$(tplink_pharos_get_model_string)"
	local line

	# Here $1 is given to dd directly instead of get_image as otherwise the skip
	# will take almost a second (as dd can't seek then)
	#
	# This will fail if the image isn't local, but that's fine: as the
	# read loop won't be executed at all, it will return true, so the image
	# is accepted (loading the first 1.5M of a remote image for this check seems
	# a bit extreme)
	dd if="$1" bs=1 skip=1511432 count=1024 2>/dev/null | while read line; do
		[ "$line" = "$model_string" ] && break
	done || {
		echo "Unsupported image (model not in support-list)"
		return 1
	}

	return 0
}

seama_get_type_magic() {
	get_image "$@" | dd bs=1 count=4 skip=53 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'
}

wrgg_get_image_magic() {
	get_image "$@" | dd bs=4 count=1 skip=8 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'
}

cybertan_get_image_magic() {
	get_image "$@" | dd bs=8 count=1 skip=0  2>/dev/null | hexdump -v -n 8 -e '1/1 "%02x"'
}

cybertan_check_image() {
	local magic="$(cybertan_get_image_magic "$1")"
	local fw_magic="$(cybertan_get_hw_magic)"

	[ "$fw_magic" != "$magic" ] && {
		echo "Invalid image, ID mismatch, got:$magic, but need:$fw_magic"
		return 1
	}

	return 0
}

platform_do_upgrade_compex() {
	local fw_file=$1
	local fw_part=$PART_NAME
	local fw_mtd=$(find_mtd_part $fw_part)
	local fw_length=0x$(dd if="$fw_file" bs=2 skip=1 count=4 2>/dev/null)
	local fw_blocks=$(($fw_length / 65536))

	if [ -n "$fw_mtd" ] &&  [ ${fw_blocks:-0} -gt 0 ]; then
		local append=""
		[ -f "$CONF_TAR" -a "$SAVE_CONFIG" -eq 1 ] && append="-j $CONF_TAR"

		sync
		dd if="$fw_file" bs=64k skip=1 count=$fw_blocks 2>/dev/null | \
			mtd $append write - "$fw_part"
	fi
}

alfa_check_image() {
	local magic_long="$(get_magic_long "$1")"
	local fw_part_size=$(mtd_get_part_size firmware)

	case "$magic_long" in
	"27051956")
		[ "$fw_part_size" != "16318464" ] && {
			echo "Invalid image magic \"$magic_long\" for $fw_part_size bytes"
			return 1
		}
		;;
	"68737173")
		[ "$fw_part_size" != "7929856" ] && {
			echo "Invalid image magic \"$magic_long\" for $fw_part_size bytes"
			return 1
		}
		;;
	esac

	return 0
}

platform_check_image() {
	local board=$(board_name)
	local magic="$(get_magic_word "$1")"
	local magic_long="$(get_magic_long "$1")"

	[ "$#" -gt 1 ] && return 1

	case "$board" in
	"ubnt,unifi")
		[ "$magic" != "2705" ] && {
			echo "Invalid image type."
			return 1
		}

		return 0
		;;
	esac

	echo "Sysupgrade is not yet supported on $board."
	return 1
}

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	*)
		default_do_upgrade "$ARGV"
		;;
	esac
}

disable_watchdog() {
	killall watchdog
	( ps | grep -v 'grep' | grep '/dev/watchdog' ) && {
		echo 'Could not disable watchdog'
		return 1
	}
}

append sysupgrade_pre_upgrade disable_watchdog
