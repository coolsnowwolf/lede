CI_BLKSZ=65536
CI_LDADR=0x00800000

platform_find_partitions() {
	local first dev size erasesize name
	while read dev size erasesize name; do
		name=${name#'"'}; name=${name%'"'}
		case "$name" in
			vmlinux.bin.l7|kernel|linux|rootfs)
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
			vmlinux.bin.l7|kernel|linux)
				echo "$part"
				break
			;;
		esac
	done
}

platform_find_part_size() {
	local first dev size erasesize name
	while read dev size erasesize name; do
		name=${name#'"'}; name=${name%'"'}
		[ "$name" = "$1" ] && {
			echo "$size"
			break
		}
	done < /proc/mtd
}

platform_do_upgrade_combined() {
	local partitions=$(platform_find_partitions)
	local kernelpart=$(platform_find_kernelpart "${partitions#*:}")
	local erase_size=$((0x${partitions%%:*})); partitions="${partitions#*:}"
	local kern_part_size=0x$(platform_find_part_size "$kernelpart")
	local kern_part_blocks=$(($kern_part_size / $CI_BLKSZ))
	local kern_length=0x$(dd if="$1" bs=2 skip=1 count=4 2>/dev/null)
	local kern_blocks=$(($kern_length / $CI_BLKSZ))
	local root_blocks=$((0x$(dd if="$1" bs=2 skip=5 count=4 2>/dev/null) / $CI_BLKSZ))

	v "platform_do_upgrade_combined"
	v "partitions=$partitions"
	v "kernelpart=$kernelpart"
	v "kernel_part_size=$kern_part_size"
	v "kernel_part_blocks=$kern_part_blocks"
	v "kern_length=$kern_length"
	v "erase_size=$erase_size"
	v "kern_blocks=$kern_blocks"
	v "root_blocks=$root_blocks"
	v "kern_pad_blocks=$(($kern_part_blocks-$kern_blocks))"

	if [ -n "$partitions" ] && [ -n "$kernelpart" ] && \
	   [ ${kern_blocks:-0} -gt 0 ] && \
	   [ ${root_blocks:-0} -gt 0 ] && \
	   [ ${erase_size:-0} -gt 0 ];
	then
		local append=""
		[ -f "$CONF_TAR" -a "$SAVE_CONFIG" -eq 1 ] && append="-j $CONF_TAR"

		# write the kernel
		dd if="$1" bs=$CI_BLKSZ skip=1 count=$kern_blocks 2>/dev/null | \
			mtd -F$kernelpart:$kern_part_size:$CI_LDADR write - $kernelpart
		# write the rootfs
		dd if="$1" bs=$CI_BLKSZ skip=$((1+$kern_blocks)) count=$root_blocks 2>/dev/null | \
			mtd $append write - rootfs
	else
		echo "invalid image"
	fi
}

platform_check_image() {
	local board=$(board_name)
	local magic="$(get_magic_word "$1")"
	local partitions=$(platform_find_partitions)
	local kernelpart=$(platform_find_kernelpart "${partitions#*:}")
	local kern_part_size=0x$(platform_find_part_size "$kernelpart")
	local kern_length=0x$(dd if="$1" bs=2 skip=1 count=4 2>/dev/null)

	[ "$#" -gt 1 ] && return 1

	case "$board" in
	avila | cambria )
		[ "$magic" != "4349" ] && {
			echo "Invalid image. Use *-sysupgrade.bin files on this board"
			return 1
		}

		kern_length_b=$(printf '%d' $kern_length)
		kern_part_size_b=$(printf '%d' $kern_part_size)
		if [ $kern_length_b -gt $kern_part_size_b ]; then
			echo "Invalid image. Kernel size ($kern_length) exceeds kernel partition ($kern_part_size)"
			return 1
		fi

		local md5_img=$(dd if="$1" bs=2 skip=9 count=16 2>/dev/null)
		local md5_chk=$(dd if="$1" bs=$CI_BLKSZ skip=1 2>/dev/null | md5sum -); md5_chk="${md5_chk%% *}"
		if [ -n "$md5_img" -a -n "$md5_chk" ] && [ "$md5_img" = "$md5_chk" ]; then
			return 0
		else
			echo "Invalid image. Contents do not match checksum (image:$md5_img calculated:$md5_chk)"
			return 1
		fi

		return 0
		;;
	esac

	echo "Sysupgrade is not yet supported on $board."
	return 1
}

platform_do_upgrade() {
	local board=$(board_name)

	v "board=$board"
	case "$board" in
	avila | cambria )
		platform_do_upgrade_combined "$ARGV"
		;;
	*)
		default_do_upgrade "$ARGV"
		;;
	esac
}
