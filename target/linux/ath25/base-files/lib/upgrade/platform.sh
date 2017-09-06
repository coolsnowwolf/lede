CI_BLKSZ=65536
CI_LDADR=0x80041000

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

platform_check_image() {
	[ "$#" -gt 1 ] && return 1

	case "$(get_magic_word "$1")" in
		# Combined Image
		4349)
			local md5_img=$(dd if="$1" bs=2 skip=9 count=16 2>/dev/null)
			local md5_chk=$(dd if="$1" bs=$CI_BLKSZ skip=1 2>/dev/null | md5sum -); md5_chk="${md5_chk%% *}"

			if [ -n "$md5_img" -a -n "$md5_chk" ] && [ "$md5_img" = "$md5_chk" ]; then
				return 0
			else
				echo "Invalid image. Contents do not match checksum (image:$md5_img calculated:$md5_chk)"
				return 1
			fi
		;;
		*)
			echo "Invalid image. Use combined .img files on this platform"
			return 1
		;;
	esac
}

platform_do_upgrade() {
	local partitions=$(platform_find_partitions)
	local kernelpart=$(platform_find_kernelpart "${partitions#*:}")
	local erase_size=$((0x${partitions%%:*})); partitions="${partitions#*:}"
	local kern_length=0x$(dd if="$1" bs=2 skip=1 count=4 2>/dev/null)
	local kern_blocks=$(($kern_length / $CI_BLKSZ))
	local root_blocks=$((0x$(dd if="$1" bs=2 skip=5 count=4 2>/dev/null) / $CI_BLKSZ))

	if [ -n "$partitions" ] && [ -n "$kernelpart" ] && \
	   [ ${kern_blocks:-0} -gt 0 ] && \
	   [ ${root_blocks:-0} -gt ${kern_blocks:-0} ] && \
	   [ ${erase_size:-0} -gt 0 ];
	then
		local append=""
		[ -f "$CONF_TAR" -a "$SAVE_CONFIG" -eq 1 ] && append="-j $CONF_TAR"

		( dd if="$1" bs=$CI_BLKSZ skip=1 count=$kern_blocks 2>/dev/null; \
		  dd if="$1" bs=$CI_BLKSZ skip=$((1+$kern_blocks)) count=$root_blocks 2>/dev/null ) | \
			mtd -r $append -F$kernelpart:$kern_length:$CI_LDADR,rootfs write - $partitions
	fi
}
