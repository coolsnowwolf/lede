. /lib/functions.sh

fortinet_bswap32() {
	local val="$(printf %08x $(($1)))"

	# swap and print in hex
	echo "0x${val:6:2}${val:4:2}${val:2:2}${val:0:2}"
}

fortinet_by2bl() {
	local blks="$(($1 / 0x200))"
	[ $(($1 % 0x200)) -gt 0 ] && blks=$((blks + 1))

	printf "0x%08x" $blks
}

fortinet_bl2by() {
	printf "0x%08x" $(($1 * 0x200))
}

fortinet_build_partmap() {
	local new="$1" old="$2"
	local len="${old%%@*}" ofs="${old##*@}"

	case "$new" in
	@*)  ofs="$(fortinet_by2bl ${new##@})" ;;	# "@<offset>"

	*@*) len="$(fortinet_by2bl ${new%%@*})"		# "<length>@<offset>"
	     ofs="$(fortinet_by2bl ${new##*@})" ;;

	"")  ;;						# "" (empty)

	*)   len="$(fortinet_by2bl ${new%%@*})" ;;	# "<length>"
	esac

	# print N blocks of length/offset in dec
	echo "${len}@${ofs}"
}

# Update firmware information in "firmware-info" partition
#
# parameters:
#   $1: image index (0/1)
#   $2: new image name (up to 32 characters)
#   $3: length and/or offset for kernel (bytes)
#   $4: length and/or offset for rootfs (bytes)
#
# Note: $3 and $4 support multiple formats:
#
# - <length>@<offset>: set <length> and <rootfs>
# - <length>         : set <length> and keep the current offset
# - @<offset>        : set <offset> and keep the current length
# - "" (empty)       : keep the current length and offset
fortinet_update_fwinfo() {
	local fwinfo_mtd="$(find_mtd_part firmware-info)"
	local index="$1"
	local name="$2"
	local offset
	local old_kr
	local old new tmp part pos
	local output

	if [ -z "$fwinfo_mtd" ]; then
		echo "ERROR: MTD device \"firmware-info\" not found"
		return 1
	fi

	# Image Name
	case "$index" in
	0) offset=0x10 ;;
	1) offset=0x30 ;;
	*) echo "ERROR: invalid image index specified!"; return 1 ;;
	esac

	printf "Image Index: %d\n" $index

	old="$(dd bs=16 count=2 skip=$((offset / 16)) if=$fwinfo_mtd 2>/dev/null)"
	printf "Image Name : \"%s\"\n" "$old"
	if [ -n "$name" ]; then
		echo -n "$name" | \
			dd bs=32 count=1 oflag=seek_bytes seek=$((offset)) \
				conv=sync,notrunc of=$fwinfo_mtd 2>/dev/null
		printf "             --> \"%s\"\n\n" "$name"
	else
		printf "\n"
	fi

	# length/offset values of kernel/rootfs
	case "$index" in
	0) offset=0x180 ;;
	1) offset=0x190 ;;
	esac

	# <kernel offset:4><kernel length:4><rootfs offset:4><rootfs length:4>
	old_kr="$(hexdump -n 16 -v -s $((offset)) -e '1/4 "%08x"' $fwinfo_mtd)"

	pos=0
	for part in kernel rootfs; do
		old="0x${old_kr:$((8 + pos)):8}@0x${old_kr:$((0 + pos)):8}"
		new="$(fortinet_build_partmap "$3" "$old")"
		shift

		printf "  %s:\n" $part
		printf "    old: 0x%08x@0x%08x\n" \
			$(fortinet_bl2by ${old%%@*}) $(fortinet_bl2by ${old##*@})
		printf "    new: 0x%08x@0x%08x\n\n" \
			$(fortinet_bl2by ${new%%@*}) $(fortinet_bl2by ${new##*@})

		tmp="$(fortinet_bswap32 ${new%%@*})@$(fortinet_bswap32 ${new##*@})"
		new="$(echo $tmp | sed 's/0x\([0-9a-f]\{8\}\)@0x\([0-9a-f]\{8\}\)/\2\1/')"
		output="${output}${new}"

		pos=$((pos + 16))
	done

	data_2bin "$output" | \
		dd bs=16 count=1 seek=$((offset / 16)) conv=notrunc \
			of=$fwinfo_mtd 2>/dev/null
}

fortinet_do_upgrade() {
	local board_dir="$(tar tf "$1" | grep -m 1 '^sysupgrade-.*/$')"
	local kern_mtd="$(find_mtd_part kernel)"
	local root_mtd="$(find_mtd_part rootfs)"
	local kern_len kern_ofs root_len root_ofs
	local imgname

	board_dir="${board_dir%/}"

	if [ -z "$kern_mtd" ] || [ -z "$root_mtd" ]; then
		echo "ERROR: MTD device \"kernel\" or \"rootfs\" not found"
		umount -a
		reboot -f
	fi
	kern_ofs=$(cat /sys/class/mtd/${kern_mtd//\/dev\/mtdblock/mtd}/offset)
	root_ofs=$(cat /sys/class/mtd/${root_mtd//\/dev\/mtdblock/mtd}/offset)

	if [ -z "$kern_ofs" ] || [ -z "$root_ofs" ]; then
		echo "ERROR: failed to get offset of kernel or rootfs"
		umount -a
		reboot -f
	fi

	kern_len=$( (tar xOf "$1" "$board_dir/kernel" | wc -c) 2> /dev/null)
	root_len=$( (tar xOf "$1" "$board_dir/root" | wc -c) 2> /dev/null)

	if [ -z "$kern_len" ] || [ -z "$root_len" ]; then
		echo "ERROR: failed to get length of new kernel or rootfs"
		umount -a
		reboot -f
	fi

	# try to load and parse /tmp/sysupgrade.meta for image name
	if [ -r "/tmp/sysupgrade.meta" ]; then
		local key value

		sed -e 's/, \{1,2\}\"/\n"/g' \
		    -e 's/{ \{1,2\}/\n/g' \
		    -e 's/ \{1,2\}}/\n/g' < /tmp/sysupgrade.meta \
			> /tmp/sysupgrade.meta.tmp
		while read key value; do
			key="${key//\"/}"
			value="${value//\"/}"

			[ -z "$value" ] && continue
			case "$key" in
			dist:|\
			version:|\
			revision:) imgname="${imgname}$value " ;;
			esac
		done < /tmp/sysupgrade.meta.tmp
	else
		imgname="OpenWrt"
	fi

	fortinet_update_fwinfo 0 "${imgname%% }" \
		"${kern_len}@${kern_ofs}" "${root_len}@${root_ofs}" || {
		umount -a
		reboot -f
	}

	tar xOf "$1" "$board_dir/kernel" | \
		mtd write - "kernel"
	tar xOf "$1" "$board_dir/root" | \
		mtd ${UPGRADE_BACKUP:+-j "${UPGRADE_BACKUP}"} write - "rootfs"
}
