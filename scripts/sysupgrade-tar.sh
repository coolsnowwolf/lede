#!/bin/sh

board=""
kernel=""
rootfs=""
outfile=""
err=""

while [ "$1" ]; do
	case "$1" in
	"--board")
		board="$2"
		shift
		shift
		continue
		;;
	"--kernel")
		kernel="$2"
		shift
		shift
		continue
		;;
	"--rootfs")
		rootfs="$2"
		shift
		shift
		continue
		;;
	*)
		if [ ! "$outfile" ]; then
			outfile=$1
			shift
			continue
		fi
		;;
	esac
done

if [ ! -n "$board" -o ! -r "$kernel" -a  ! -r "$rootfs" -o ! "$outfile" ]; then
	echo "syntax: $0 [--board boardname] [--kernel kernelimage] [--rootfs rootfs] out"
	exit 1
fi

tmpdir="$( mktemp -d 2> /dev/null )"
if [ -z "$tmpdir" ]; then
	# try OSX signature
	tmpdir="$( mktemp -t 'ubitmp' -d )"
fi

if [ -z "$tmpdir" ]; then
	exit 1
fi

mkdir -p "${tmpdir}/sysupgrade-${board}"
echo "BOARD=${board}" > "${tmpdir}/sysupgrade-${board}/CONTROL"
[ -z "${rootfs}" ] || cp "${rootfs}" "${tmpdir}/sysupgrade-${board}/root"
[ -z "${kernel}" ] || cp "${kernel}" "${tmpdir}/sysupgrade-${board}/kernel"

mtime=""
if [ -n "$SOURCE_DATE_EPOCH" ]; then
	mtime="--mtime=@${SOURCE_DATE_EPOCH}"
fi

(cd "$tmpdir"; tar cvf sysupgrade.tar sysupgrade-${board} ${mtime})
err="$?"
if [ -e "$tmpdir/sysupgrade.tar" ]; then
	cp "$tmpdir/sysupgrade.tar" "$outfile"
else
	err=2
fi
rm -rf "$tmpdir"

exit $err
