#!/bin/sh

. $TOPDIR/scripts/functions.sh

part=""
ubootenv=""
ubinize_param=""
kernel=""
rootfs=""
outfile=""
err=""
ubinize_seq=""

ubivol() {
	volid=$1
	name=$2
	image=$3
	autoresize=$4
	size="$5"
	echo "[$name]"
	echo "mode=ubi"
	echo "vol_id=$volid"
	echo "vol_type=dynamic"
	echo "vol_name=$name"
	if [ "$image" ]; then
		echo "image=$image"
		[ -n "$size" ] && echo "vol_size=${size}"
	else
		echo "vol_size=1MiB"
	fi
	if [ "$autoresize" ]; then
		echo "vol_flags=autoresize"
	fi
}

ubilayout() {
	local vol_id=0
	local rootsize=
	local autoresize=
	local rootfs_type="$( get_fs_type "$2" )"

	if [ "$1" = "ubootenv" ]; then
		ubivol $vol_id ubootenv
		vol_id=$(( $vol_id + 1 ))
		ubivol $vol_id ubootenv2
		vol_id=$(( $vol_id + 1 ))
	fi
	for part in $parts; do
		name="${part%%=*}"
		prev="$part"
		part="${part#*=}"
		[ "$prev" = "$part" ] && part=

		image="${part%%=*}"
		prev="$part"
		part="${part#*=}"
		[ "$prev" = "$part" ] && part=

		size="$part"

		ubivol $vol_id "$name" "$image" "" "${size}MiB"
		vol_id=$(( $vol_id + 1 ))
	done
	if [ "$3" ]; then
		ubivol $vol_id kernel "$3"
		vol_id=$(( $vol_id + 1 ))
	fi

	case "$rootfs_type" in
	"ubifs")
		autoresize=1
		;;
	"squashfs")
		# squashfs uses 1k block size, ensure we do not
		# violate that
		rootsize="$( round_up "$( stat -c%s "$2" )" 1024 )"
		;;
	esac
	ubivol $vol_id rootfs "$2" "$autoresize" "$rootsize"

	vol_id=$(( $vol_id + 1 ))
	[ "$rootfs_type" = "ubifs" ] || ubivol $vol_id rootfs_data "" 1
}

set_ubinize_seq() {
	if [ -n "$SOURCE_DATE_EPOCH" ] ; then
		ubinize_seq="-Q $SOURCE_DATE_EPOCH"
	fi
}

while [ "$1" ]; do
	case "$1" in
	"--uboot-env")
		ubootenv="ubootenv"
		shift
		continue
		;;
	"--kernel")
		kernel="$2"
		shift
		shift
		continue
		;;
	"--part")
		parts="$parts $2"
		shift
		shift
		continue
		;;
	"-"*)
		ubinize_param="$@"
		break
		;;
	*)
		if [ ! "$rootfs" ]; then
			rootfs=$1
			shift
			continue
		fi
		if [ ! "$outfile" ]; then
			outfile=$1
			shift
			continue
		fi
		;;
	esac
done

if [ ! -r "$rootfs" -o ! -r "$kernel" -a ! "$outfile" ]; then
	echo "syntax: $0 [--uboot-env] [--part <name>=<file>] [--kernel kernelimage] rootfs out [ubinize opts]"
	exit 1
fi

ubinize="$( which ubinize )"
if [ ! -x "$ubinize" ]; then
	echo "ubinize tool not found or not usable"
	exit 1
fi

ubinizecfg="$( mktemp 2> /dev/null )"
if [ -z "$ubinizecfg" ]; then
	# try OSX signature
	ubinizecfg="$( mktemp -t 'ubitmp' )"
fi
ubilayout "$ubootenv" "$rootfs" "$kernel" > "$ubinizecfg"

set_ubinize_seq
cat "$ubinizecfg"
ubinize $ubinize_seq -o "$outfile" $ubinize_param "$ubinizecfg"
err="$?"
[ ! -e "$outfile" ] && err=2
rm "$ubinizecfg"

exit $err
