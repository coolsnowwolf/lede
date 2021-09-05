#!/bin/sh

cmd="$1"
shift

if [ "$cmd" = "name" ]; then
	echo "UBI"
	return 0
fi

test -e /sys/class/ubi/version || return 0
read -r ubiver < /sys/class/ubi/version
[ "$ubiver" = "1" ] || return 1
test -e /sys/devices/virtual/ubi || return 0

ubidev=$(ls -1 /sys/devices/virtual/ubi | head -n 1)

read -r ebsize < "/sys/devices/virtual/ubi/${ubidev}/eraseblock_size"

freebytes() {
	read -r availeb < "/sys/devices/virtual/ubi/${ubidev}/avail_eraseblocks"
	echo $((availeb * ebsize))
}

totalbytes() {
	read -r totaleb < "/sys/devices/virtual/ubi/${ubidev}/total_eraseblocks"
	echo $((totaleb * ebsize))
}

getdev() {
	local voldir volname
	for voldir in "/sys/devices/virtual/ubi/${ubidev}/${ubidev}_"*; do
		read -r volname < "${voldir}/name"
		case "$volname" in
			uvol-[rw][owpd]-$1)
				basename "$voldir"
				break
				;;
			*)
				continue
				;;
		esac
	done
}

vol_is_mode() {
	local voldev="$1"
	local volname
	read -r volname < "/sys/devices/virtual/ubi/${ubidev}/${voldev}/name"
	case "$volname" in
		uvol-$2-*)
			return 0
			;;
	esac
	return 1
}

getstatus() {
	local voldev
	voldev="$(getdev "$@")"
	[ "$voldev" ] || return 2
	vol_is_mode "$voldev" wo && return 22
	vol_is_mode "$voldev" wp && return 16
	vol_is_mode "$voldev" wd && return 1
	vol_is_mode "$voldev" ro && [ ! -e "/dev/ubiblock${voldev:3}" ] && return 1
	return 0
}

getsize() {
	local voldev
	voldev="$(getdev "$@")"
	[ "$voldev" ] || return 2
	cat "/sys/devices/virtual/ubi/${ubidev}/${voldev}/data_bytes"
}

getuserdev() {
	local voldev
	voldev="$(getdev "$@")"
	[ "$voldev" ] || return 2
	if vol_is_mode "$voldev" ro ; then
		echo "/dev/ubiblock${voldev:3}"
	elif vol_is_mode "$voldev" rw ; then
		echo "/dev/$voldev"
	fi
}

mkubifs() {
	local tmp_mp
	tmp_mp="$(mktemp -d)"
	mount -t ubifs "$1" "$tmp_mp"
	umount "$tmp_mp"
	rmdir "$tmp_mp"
}

createvol() {
	local mode ret voldev
	voldev=$(getdev "$@")
	[ "$voldev" ] && return 17
	case "$3" in
		ro|wo)
			mode=wo
			;;
		rw)
			mode=wp
			;;
		*)
			return 22
			;;
	esac
	ubimkvol "/dev/$ubidev" -N "uvol-$mode-$1" -s "$2"
	ret=$?
	[ $ret -eq 0 ] || return $ret
	voldev="$(getdev "$@")"
	ubiupdatevol -t "/dev/$voldev"
	[ "$mode" = "wp" ] || return 0
	mkubifs "/dev/$voldev"
	ubirename "/dev/$ubidev" "uvol-wp-$1" "uvol-rw-$1"
	ubus send block.volume "{\"name\": \"$1\", \"action\": \"up\", \"mode\": \"rw\", \"fstype\": \"ubifs\", \"device\": \"/dev/$voldev\"}"
}

removevol() {
	local voldev evdata
	voldev=$(getdev "$@")
	[ "$voldev" ] || return 2
	if vol_is_mode "$voldev" rw ; then
		evdata="{\"name\": \"$1\", \"action\": \"down\", \"device\": \"/dev/$voldev\"}"
	elif vol_is_mode "$voldev" ro && [ -e "/dev/ubiblock${voldev:3}" ]; then
		evdata="{\"name\": \"$1\", \"action\": \"down\", \"device\": \"/dev/ubiblock${voldev:3}\"}"
	fi
	local volnum="${voldev#${ubidev}_}"
	ubirmvol "/dev/$ubidev" -n "$volnum" || return $?
	[ "$evdata" ] && ubus send block.volume "$evdata"
}

activatevol() {
	local voldev
	voldev="$(getdev "$@")"
	[ "$voldev" ] || return 2
	vol_is_mode "$voldev" rw && return 0
	vol_is_mode "$voldev" wo && return 22
	vol_is_mode "$voldev" wp && return 16
	if vol_is_mode "$voldev" ro; then
		[ -e "/dev/ubiblock${voldev:3}" ] && return 0
		ubiblock --create "/dev/$voldev"
		ubus send block.volume "{\"name\": \"$1\", \"action\": \"up\", \"mode\": \"ro\", \"device\": \"/dev/ubiblock${voldev:3}\"}"
		return 0
	elif vol_is_mode "$voldev" wd; then
		ubirename "/dev/$ubidev" "uvol-wd-$1" "uvol-rw-$1"
		ubus send block.volume "{\"name\": \"$1\", \"action\": \"up\", \"mode\": \"rw\", \"fstype\": \"ubifs\", \"device\": \"/dev/$voldev\"}"
		return 0
	fi
}

disactivatevol() {
	local voldev
	voldev="$(getdev "$@")"
	[ "$voldev" ] || return 2
	vol_is_mode "$voldev" wo && return 22
	vol_is_mode "$voldev" wp && return 16
	if vol_is_mode "$voldev" ro; then
		[ -e "/dev/ubiblock${voldev:3}" ] || return 0
		ubiblock --remove "/dev/$voldev" || return $?
		ubus send block.volume "{\"name\": \"$1\", \"action\": \"down\", \"mode\": \"ro\", \"device\": \"/dev/ubiblock${voldev:3}\"}"
		return 0
	elif vol_is_mode "$voldev" rw; then
		ubirename "/dev/$ubidev" "uvol-rw-$1" "uvol-wd-$1" || return $?
		ubus send block.volume "{\"name\": \"$1\", \"action\": \"down\", \"mode\": \"rw\", \"device\": \"/dev/$voldev\"}"
		return 0
	fi
}

updatevol() {
	local voldev
	voldev="$(getdev "$@")"
	[ "$voldev" ] || return 2
	[ "$2" ] || return 22
	vol_is_mode "$voldev" wo || return 22
	ubiupdatevol -s "$2" "/dev/$voldev" -
	ubirename "/dev/$ubidev" "uvol-wo-$1" "uvol-ro-$1"
	ubiblock --create "/dev/$voldev"
	ubus send block.volume "{\"name\": \"$1\", \"action\": \"up\", \"mode\": \"ro\", \"device\": \"/dev/ubiblock${voldev:3}\"}"
}

listvols() {
	local volname volmode volsize
	for voldir in "/sys/devices/virtual/ubi/${ubidev}/${ubidev}_"*; do
		read -r volname < "$voldir/name"
		case "$volname" in
			uvol-[rw][wod]*)
				read -r volsize < "$voldir/data_bytes"
				;;
			*)
				continue
				;;
		esac
		volmode="${volname:5:2}"
		volname="${volname:8}"
		echo "$volname $volmode $volsize"
	done
}

bootvols() {
	local volname volmode volsize voldev fstype
	for voldir in "/sys/devices/virtual/ubi/${ubidev}/${ubidev}_"*; do
		read -r volname < "$voldir/name"
		voldev="$(basename "$voldir")"
		fstype=
		case "$volname" in
			uvol-ro-*)
				voldev="/dev/ubiblock${voldev:3}"
				ubiblock --create "/dev/$voldev"
				;;
			uvol-rw-*)
				voldev="/dev/$voldev"
				fstype="ubifs"
				;;
			*)
				continue
				;;
		esac
		volmode="${volname:5:2}"
		volname="${volname:8}"
		ubus send block.volume "{\"name\": \"$volname\", \"action\": \"up\", \"mode\": \"$volmode\",${fstype:+ \"fstype\": \"$fstype\", }\"device\": \"$voldev\"}"
	done
}

case "$cmd" in
	align)
		echo "$ebsize"
		;;
	free)
		freebytes
		;;
	total)
		totalbytes
		;;
	boot)
		bootvols
		;;
	list)
		listvols "$@"
		;;
	create)
		createvol "$@"
		;;
	remove)
		removevol "$@"
		;;
	device)
		getuserdev "$@"
		;;
	size)
		getsize "$@"
		;;
	up)
		activatevol "$@"
		;;
	down)
		disactivatevol "$@"
		;;
	status)
		getstatus "$@"
		;;
	write)
		updatevol "$@"
		;;
	*)
		echo "unknown command"
		return 1
		;;
esac
