#!/bin/sh
. /lib/functions.sh
. /lib/functions/system.sh

partname=""
offset=""
NEW_MAC=
YES=

board=$(board_name)
case $board in
	hatlab,gateboard-one)
		partname=factory
		offset=$((0x0))
	;;
	mqmaker,witi)
		partname=factory
		offset=$((0xe000))
	;;
	*)
		echo "Unsupported board"
		exit 1
	;;
esac

while [ -n "$1" ]; do
	case "$1" in
		??:??:??:??:??:??) NEW_MAC="$1";;
		-y) YES=1;;
		*)
			cat <<EOF
Unknown option/argument '$1'
Usage: $0 [-y] [<macaddr>]
EOF
			exit 1
		;;
	esac
	shift
done

ask_bool() {
	local message="$1"
	local default="$((! ${2:-0}))"
	[ -n "$YES" ] && return 0
	echo -n "$message "
	read opt
	case "$opt" in
		y|Y) return 0;;
		n|N) return 1;;
		*) return $default;;
	esac
}

convert_hex() {
	hexdump -e '/1 "%02x "'
}

gen_mac() {
	dd if=/dev/urandom bs=6 count=1 2>/dev/null
}

mac="$(mtd_get_mac_binary $partname $offset)"
case "$mac" in
	00:00:00:00:00:00);;
	ff:ff:ff:ff:ff:ff);;
	*)
		echo "Current MAC address: $mac"
		ask_bool "Overwrite (y/N)?" 0 || exit
	;;
esac

if [ -n "$NEW_MAC" ]; then
	set -- $(echo "$NEW_MAC" | sed 's,:, ,g')
else
	set -- $(gen_mac | convert_hex)
	set -- $(printf %02x $(( (0x$1 & 0xfe) | 0x02 ))) $2 $3 $4 $5 $6
fi
echo "New MAC address: $1:$2:$3:$4:$5:$6"
ask_bool "Write to EEPROM (y/N)?" || exit

part=$(find_mtd_part "$partname")
[ -n "$part" ] || exit
echo -ne "\x$1\x$2\x$3\x$4\x$5\x$6" | dd of=$part conv=notrunc bs=1 count=6 seek=$offset 2>/dev/null
echo "Done"
