# Copyright (C) 2006-2013 OpenWrt.org

get_mac_binary() {
	local path="$1"
	local offset="$2"

	if [ -z "$path" ]; then
		echo "get_mac_binary: file $path not found!" >&2
		return
	fi

	hexdump -v -n 6 -s $offset -e '5/1 "%02x:" 1/1 "%02x"' $path 2>/dev/null
}

find_mtd_chardev() {
	local INDEX=$(find_mtd_index "$1")
	local PREFIX=/dev/mtd

	[ -d /dev/mtd ] && PREFIX=/dev/mtd/
	echo "${INDEX:+$PREFIX$INDEX}"
}

mtd_get_mac_ascii()
{
	local mtdname="$1"
	local key="$2"
	local part
	local mac_dirty

	part=$(find_mtd_part "$mtdname")
	if [ -z "$part" ]; then
		echo "mtd_get_mac_ascii: partition $mtdname not found!" >&2
		return
	fi

	mac_dirty=$(strings "$part" | sed -n 's/^'"$key"'=//p')

	# "canonicalize" mac
	[ -n "$mac_dirty" ] && macaddr_canonicalize "$mac_dirty"
}

mtd_get_mac_binary() {
	local mtdname="$1"
	local offset="$2"
	local part

	part=$(find_mtd_part "$mtdname")
	get_mac_binary "$part" "$offset"
}

mtd_get_mac_binary_ubi() {
	local mtdname="$1"
	local offset="$2"

	. /lib/upgrade/nand.sh

	local ubidev=$(nand_find_ubi $CI_UBIPART)
	local part=$(nand_find_volume $ubidev $1)

	if [ -z "$part" ]; then
		echo "mtd_get_mac_binary: ubi volume $mtdname not found!" >&2
		return
	fi

	hexdump -v -n 6 -s $offset -e '5/1 "%02x:" 1/1 "%02x"' /dev/$part 2>/dev/null
}

mtd_get_part_size() {
	local part_name=$1
	local first dev size erasesize name
	while read dev size erasesize name; do
		name=${name#'"'}; name=${name%'"'}
		if [ "$name" = "$part_name" ]; then
			echo $((0x$size))
			break
		fi
	done < /proc/mtd
}

macaddr_add() {
	local mac=$1
	local val=$2
	local oui=${mac%:*:*:*}
	local nic=${mac#*:*:*:}

	nic=$(printf "%06x" $((0x${nic//:/} + $val & 0xffffff)) | sed 's/^\(.\{2\}\)\(.\{2\}\)\(.\{2\}\)/\1:\2:\3/')
	echo $oui:$nic
}

macaddr_setbit_la()
{
	local mac=$1

	printf "%02x:%s" $((0x${mac%%:*} | 0x02)) ${mac#*:}
}

macaddr_2bin()
{
	local mac=$1

	echo -ne \\x${mac//:/\\x}
}

macaddr_canonicalize()
{
	local mac="$1"
	local canon=""

	mac=$(echo -n $mac | tr -d \")
	[ ${#mac} -gt 17 ] && return
	[ -n "${mac//[a-fA-F0-9\.: -]/}" ] && return

	for octet in ${mac//[\.:-]/ }; do
		case "${#octet}" in
		1)
			octet="0${octet}"
			;;
		2)
			;;
		4)
			octet="${octet:0:2} ${octet:2:2}"
			;;
		12)
			octet="${octet:0:2} ${octet:2:2} ${octet:4:2} ${octet:6:2} ${octet:8:2} ${octet:10:2}"
			;;
		*)
			return
			;;
		esac
		canon=${canon}${canon:+ }${octet}
	done

	[ ${#canon} -ne 17 ] && return

	printf "%02x:%02x:%02x:%02x:%02x:%02x" 0x${canon// / 0x} 2>/dev/null
}
