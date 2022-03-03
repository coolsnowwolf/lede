RAM_ROOT=/tmp/root

export BACKUP_FILE=sysupgrade.tgz	# file extracted by preinit

[ -x /usr/bin/ldd ] || ldd() { LD_TRACE_LOADED_OBJECTS=1 $*; }
libs() { ldd $* 2>/dev/null | sed -E 's/(.* => )?(.*) .*/\2/'; }

install_file() { # <file> [ <file> ... ]
	local target dest dir
	for file in "$@"; do
		if [ -L "$file" ]; then
			target="$(readlink -f "$file")"
			dest="$RAM_ROOT/$file"
			[ ! -f "$dest" ] && {
				dir="$(dirname "$dest")"
				mkdir -p "$dir"
				ln -s "$target" "$dest"
			}
			file="$target"
		fi
		dest="$RAM_ROOT/$file"
		[ -f "$file" -a ! -f "$dest" ] && {
			dir="$(dirname "$dest")"
			mkdir -p "$dir"
			cp "$file" "$dest"
		}
	done
}

install_bin() {
	local src files
	src=$1
	files=$1
	[ -x "$src" ] && files="$src $(libs $src)"
	install_file $files
}

run_hooks() {
	local arg="$1"; shift
	for func in "$@"; do
		eval "$func $arg"
	done
}

ask_bool() {
	local default="$1"; shift;
	local answer="$default"

	[ "$INTERACTIVE" -eq 1 ] && {
		case "$default" in
			0) echo -n "$* (y/N): ";;
			*) echo -n "$* (Y/n): ";;
		esac
		read answer
		case "$answer" in
			y*) answer=1;;
			n*) answer=0;;
			*) answer="$default";;
		esac
	}
	[ "$answer" -gt 0 ]
}

_v() {
	[ -n "$VERBOSE" ] && [ "$VERBOSE" -ge 1 ] && echo "$*" >&2
}

_vn() {
	[ -n "$VERBOSE" ] && [ "$VERBOSE" -ge 1 ] && echo -n "$*" >&2
}

v() {
	_v "$(date) upgrade: $@"
}

vn() {
	_vn "$(date) upgrade: $@"
}

json_string() {
	local v="$1"
	v="${v//\\/\\\\}"
	v="${v//\"/\\\"}"
	echo "\"$v\""
}

rootfs_type() {
	/bin/mount | awk '($3 ~ /^\/$/) && ($5 !~ /rootfs/) { print $5 }'
}

get_image() { # <source> [ <command> ]
	local from="$1"
	local cmd="$2"

	if [ -z "$cmd" ]; then
		local magic="$(dd if="$from" bs=2 count=1 2>/dev/null | hexdump -n 2 -e '1/1 "%02x"')"
		case "$magic" in
			1f8b) cmd="zcat";;
			425a) cmd="bzcat";;
			*) cmd="cat";;
		esac
	fi

	$cmd <"$from"
}

get_image_dd() {
	local from="$1"; shift

	(
		exec 3>&2
		( exec 3>&2; get_image "$from" 2>&1 1>&3 | grep -v -F ' Broken pipe'     ) 2>&1 1>&3 \
			| ( exec 3>&2; dd "$@" 2>&1 1>&3 | grep -v -E ' records (in|out)') 2>&1 1>&3
		exec 3>&-
	)
}

get_magic_word() {
	(get_image "$@" | dd bs=2 count=1 | hexdump -v -n 2 -e '1/1 "%02x"') 2>/dev/null
}

get_magic_long() {
	(get_image "$@" | dd bs=4 count=1 | hexdump -v -n 4 -e '1/1 "%02x"') 2>/dev/null
}

get_magic_gpt() {
	(get_image "$@" | dd bs=8 count=1 skip=64) 2>/dev/null
}

get_magic_vfat() {
	(get_image "$@" | dd bs=3 count=1 skip=18) 2>/dev/null
}

get_magic_fat32() {
	(get_image "$@" | dd bs=1 count=5 skip=82) 2>/dev/null
}

part_magic_efi() {
	local magic=$(get_magic_gpt "$@")
	[ "$magic" = "EFI PART" ]
}

part_magic_fat() {
	local magic=$(get_magic_vfat "$@")
	local magic_fat32=$(get_magic_fat32 "$@")
	[ "$magic" = "FAT" ] || [ "$magic_fat32" = "FAT32" ]
}

export_bootdevice() {
	local cmdline uuid blockdev uevent line class
	local MAJOR MINOR DEVNAME DEVTYPE
	local rootpart="$(cmdline_get_var root)"

	case "$rootpart" in
		PARTUUID=[a-f0-9][a-f0-9][a-f0-9][a-f0-9][a-f0-9][a-f0-9][a-f0-9][a-f0-9]-[a-f0-9][a-f0-9])
			uuid="${rootpart#PARTUUID=}"
			uuid="${uuid%-[a-f0-9][a-f0-9]}"
			for blockdev in $(find /dev -type b); do
				set -- $(dd if=$blockdev bs=1 skip=440 count=4 2>/dev/null | hexdump -v -e '4/1 "%02x "')
				if [ "$4$3$2$1" = "$uuid" ]; then
					uevent="/sys/class/block/${blockdev##*/}/uevent"
					break
				fi
			done
		;;
		PARTUUID=????????-????-????-????-??????????02)
			uuid="${rootpart#PARTUUID=}"
			uuid="${uuid%02}00"
			for disk in $(find /dev -type b); do
				set -- $(dd if=$disk bs=1 skip=568 count=16 2>/dev/null | hexdump -v -e '8/1 "%02x "" "2/1 "%02x""-"6/1 "%02x"')
				if [ "$4$3$2$1-$6$5-$8$7-$9" = "$uuid" ]; then
					uevent="/sys/class/block/${disk##*/}/uevent"
					break
				fi
			done
		;;
		/dev/*)
			uevent="/sys/class/block/${rootpart##*/}/../uevent"
		;;
		0x[a-f0-9][a-f0-9][a-f0-9] | 0x[a-f0-9][a-f0-9][a-f0-9][a-f0-9] | \
		[a-f0-9][a-f0-9][a-f0-9] | [a-f0-9][a-f0-9][a-f0-9][a-f0-9])
			rootpart=0x${rootpart#0x}
			for class in /sys/class/block/*; do
				while read line; do
					export -n "$line"
				done < "$class/uevent"
				if [ $((rootpart/256)) = $MAJOR -a $((rootpart%256)) = $MINOR ]; then
					uevent="$class/../uevent"
				fi
			done
		;;
	esac

	if [ -e "$uevent" ]; then
		while read line; do
			export -n "$line"
		done < "$uevent"
		export BOOTDEV_MAJOR=$MAJOR
		export BOOTDEV_MINOR=$MINOR
		return 0
	fi

	return 1
}

export_partdevice() {
	local var="$1" offset="$2"
	local uevent line MAJOR MINOR DEVNAME DEVTYPE

	for uevent in /sys/class/block/*/uevent; do
		while read line; do
			export -n "$line"
		done < "$uevent"
		if [ $BOOTDEV_MAJOR = $MAJOR -a $(($BOOTDEV_MINOR + $offset)) = $MINOR -a -b "/dev/$DEVNAME" ]; then
			export "$var=$DEVNAME"
			return 0
		fi
	done

	return 1
}

hex_le32_to_cpu() {
	[ "$(echo 01 | hexdump -v -n 2 -e '/2 "%x"')" = "3031" ] && {
		echo "${1:0:2}${1:8:2}${1:6:2}${1:4:2}${1:2:2}"
		return
	}
	echo "$@"
}

get_partition_by_name() {
	for partname in /sys/class/block/$1/*/name; do
		[ "$(cat ${partname})" = "$2" ] && {
			basename ${partname%%/name}
			break
		}
	done
}

get_partitions() { # <device> <filename>
	local disk="$1"
	local filename="$2"

	if [ -b "$disk" -o -f "$disk" ]; then
		v "Reading partition table from $filename..."

		local magic=$(dd if="$disk" bs=2 count=1 skip=255 2>/dev/null)
		if [ "$magic" != $'\x55\xAA' ]; then
			v "Invalid partition table on $disk"
			exit
		fi

		rm -f "/tmp/partmap.$filename"

		local part
		part_magic_efi "$disk" && {
			#export_partdevice will fail when partition number is greater than 15, as
			#the partition major device number is not equal to the disk major device number
			for part in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15; do
				set -- $(hexdump -v -n 48 -s "$((0x380 + $part * 0x80))" -e '4/4 "%08x"" "4/4 "%08x"" "4/4 "0x%08X "' "$disk")

				local type="$1"
				local lba="$(( $(hex_le32_to_cpu $4) * 0x100000000 + $(hex_le32_to_cpu $3) ))"
				local end="$(( $(hex_le32_to_cpu $6) * 0x100000000 + $(hex_le32_to_cpu $5) ))"
				local num="$(( $end - $lba + 1 ))"

				[ "$type" = "00000000000000000000000000000000" ] && continue

				printf "%2d %5d %7d\n" $part $lba $num >> "/tmp/partmap.$filename"
			done
		} || {
			for part in 1 2 3 4; do
				set -- $(hexdump -v -n 12 -s "$((0x1B2 + $part * 16))" -e '3/4 "0x%08X "' "$disk")

				local type="$(( $(hex_le32_to_cpu $1) % 256))"
				local lba="$(( $(hex_le32_to_cpu $2) ))"
				local num="$(( $(hex_le32_to_cpu $3) ))"

				[ $type -gt 0 ] || continue

				printf "%2d %5d %7d\n" $part $lba $num >> "/tmp/partmap.$filename"
			done
		}
	fi
}

indicate_upgrade() {
	. /etc/diag.sh
	set_state upgrade
}

# Flash firmware to MTD partition
#
# $(1): path to image
# $(2): (optional) pipe command to extract firmware, e.g. dd bs=n skip=m
default_do_upgrade() {
	sync
	echo 3 > /proc/sys/vm/drop_caches
	if [ -n "$UPGRADE_BACKUP" ]; then
		get_image "$1" "$2" | mtd $MTD_ARGS $MTD_CONFIG_ARGS -j "$UPGRADE_BACKUP" write - "${PART_NAME:-image}"
	else
		get_image "$1" "$2" | mtd $MTD_ARGS write - "${PART_NAME:-image}"
	fi
	[ $? -ne 0 ] && exit 1
}
