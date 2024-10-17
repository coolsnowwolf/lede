# Copyright (C) 2006-2013 OpenWrt.org

. /lib/functions.sh
. /usr/share/libubox/jshn.sh

get_mac_binary() {
	local path="$1"
	local offset="$2"

	if ! [ -e "$path" ]; then
		echo "get_mac_binary: file $path not found!" >&2
		return
	fi

	hexdump -v -n 6 -s $offset -e '5/1 "%02x:" 1/1 "%02x"' $path 2>/dev/null
}

get_mac_label_dt() {
	local basepath="/proc/device-tree"
	local macdevice="$(cat "$basepath/aliases/label-mac-device" 2>/dev/null)"
	local macaddr

	[ -n "$macdevice" ] || return

	macaddr=$(get_mac_binary "$basepath/$macdevice/mac-address" 0 2>/dev/null)
	[ -n "$macaddr" ] || macaddr=$(get_mac_binary "$basepath/$macdevice/local-mac-address" 0 2>/dev/null)

	echo $macaddr
}

get_mac_label_json() {
	local cfg="/etc/board.json"
	local macaddr

	[ -s "$cfg" ] || return

	json_init
	json_load "$(cat $cfg)"
	if json_is_a system object; then
		json_select system
			json_get_var macaddr label_macaddr
		json_select ..
	fi

	echo $macaddr
}

get_mac_label() {
	local macaddr=$(get_mac_label_dt)

	[ -n "$macaddr" ] || macaddr=$(get_mac_label_json)

	echo $macaddr
}

find_mtd_chardev() {
	local INDEX=$(find_mtd_index "$1")
	local PREFIX=/dev/mtd

	[ -d /dev/mtd ] && PREFIX=/dev/mtd/
	echo "${INDEX:+$PREFIX$INDEX}"
}

get_mac_ascii() {
	local part="$1"
	local key="$2"
	local mac_dirty

	mac_dirty=$(strings "$part" | sed -n 's/^'"$key"'=//p')

	# "canonicalize" mac
	[ -n "$mac_dirty" ] && macaddr_canonicalize "$mac_dirty"
}

mtd_get_mac_ascii() {
	local mtdname="$1"
	local key="$2"
	local part

	part=$(find_mtd_part "$mtdname")
	if [ -z "$part" ]; then
		echo "mtd_get_mac_ascii: partition $mtdname not found!" >&2
		return
	fi

	get_mac_ascii "$part" "$key"
}

mtd_get_mac_encrypted_arcadyan() {
	local iv="00000000000000000000000000000000"
	local key="2A4B303D7644395C3B2B7053553C5200"
	local mac_dirty
	local mtdname="$1"
	local part
	local size

	part=$(find_mtd_part "$mtdname")
	if [ -z "$part" ]; then
		echo "mtd_get_mac_encrypted_arcadyan: partition $mtdname not found!" >&2
		return
	fi

	# Config decryption and getting mac. Trying uencrypt and openssl utils.
	size=$((0x$(dd if=$part skip=9 bs=1 count=4 2>/dev/null | hexdump -v -e '1/4 "%08x"')))
	if [[ -f  "/usr/bin/uencrypt" ]]; then
		mac_dirty=$(dd if=$part bs=1 count=$size skip=$((0x100)) 2>/dev/null | \
			uencrypt -d -n -k $key -i $iv | grep mac | cut -c 5-)
	elif [[ -f  "/usr/bin/openssl" ]]; then
		mac_dirty=$(dd if=$part bs=1 count=$size skip=$((0x100)) 2>/dev/null | \
			openssl aes-128-cbc -d -nopad -K $key -iv $iv | grep mac | cut -c 5-)
	else
		echo "mtd_get_mac_encrypted_arcadyan: Neither uencrypt nor openssl was found!" >&2
		return
	fi

	# "canonicalize" mac
	[ -n "$mac_dirty" ] && macaddr_canonicalize "$mac_dirty"
}

mtd_get_mac_encrypted_deco() {
	local mtdname="$1"

	if ! [ -e "$mtdname" ]; then
		echo "mtd_get_mac_encrypted_deco: file $mtdname not found!" >&2
		return
	fi

	tplink_key="3336303032384339"

	key=$(dd if=$mtdname bs=1 skip=16 count=8 2>/dev/null | \
		uencrypt -n -d -k $tplink_key -c des-ecb | hexdump -v -n 8 -e '1/1 "%02x"')

	macaddr=$(dd if=$mtdname bs=1 skip=32 count=8 2>/dev/null | \
		uencrypt -n -d -k $key -c des-ecb | hexdump -v -n 6 -e '5/1 "%02x:" 1/1 "%02x"')

	echo $macaddr
}

mtd_get_mac_uci_config_ubi() {
	local volumename="$1"

	. /lib/upgrade/nand.sh

	local ubidev=$(nand_attach_ubi $CI_UBIPART)
	local part=$(nand_find_volume $ubidev $volumename)

	cat "/dev/$part" | sed -n 's/^\s*option macaddr\s*'"'"'\?\([0-9A-F:]\+\)'"'"'\?/\1/Ip'
}

mtd_get_mac_text() {
	local mtdname="$1"
	local offset=$((${2:-0}))
	local length="${3:-17}"
	local part

	part=$(find_mtd_part "$mtdname")
	if [ -z "$part" ]; then
		echo "mtd_get_mac_text: partition $mtdname not found!" >&2
		return
	fi

	[ $((offset + length)) -le $(mtd_get_part_size "$mtdname") ] || return

	macaddr_canonicalize $(dd bs=1 if="$part" skip="$offset" count="$length" 2>/dev/null)
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

	get_mac_binary "/dev/$part" "$offset"
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

mmc_get_mac_ascii() {
	local part_name="$1"
	local key="$2"
	local part

	part=$(find_mmc_part "$part_name")
	if [ -z "$part" ]; then
		echo "mmc_get_mac_ascii: partition $part_name not found!" >&2
		return
	fi

	get_mac_ascii "$part" "$key"
}

mmc_get_mac_binary() {
	local part_name="$1"
	local offset="$2"
	local part

	part=$(find_mmc_part "$part_name")
	get_mac_binary "$part" "$offset"
}

macaddr_add() {
	local mac=$1
	local val=$2
	local oui=${mac%:*:*:*}
	local nic=${mac#*:*:*:}

	nic=$(printf "%06x" $((0x${nic//:/} + val & 0xffffff)) | sed 's/^\(.\{2\}\)\(.\{2\}\)\(.\{2\}\)/\1:\2:\3/')
	echo $oui:$nic
}

macaddr_generate_from_mmc_cid() {
	local mmc_dev=$1

	local sd_hash=$(sha256sum /sys/class/block/$mmc_dev/device/cid)
	local mac_base=$(macaddr_canonicalize "$(echo "${sd_hash}" | dd bs=1 count=12 2>/dev/null)")
	echo "$(macaddr_unsetbit_mc "$(macaddr_setbit_la "${mac_base}")")"
}

macaddr_geteui() {
	local mac=$1
	local sep=$2

	echo ${mac:9:2}$sep${mac:12:2}$sep${mac:15:2}
}

macaddr_setbit() {
	local mac=$1
	local bit=${2:-0}

	[ $bit -gt 0 -a $bit -le 48 ] || return

	printf "%012x" $(( 0x${mac//:/} | 2**(48-bit) )) | sed -e 's/\(.\{2\}\)/\1:/g' -e 's/:$//'
}

macaddr_unsetbit() {
	local mac=$1
	local bit=${2:-0}

	[ $bit -gt 0 -a $bit -le 48 ] || return

	printf "%012x" $(( 0x${mac//:/} & ~(2**(48-bit)) )) | sed -e 's/\(.\{2\}\)/\1:/g' -e 's/:$//'
}

macaddr_setbit_la() {
	macaddr_setbit $1 7
}

macaddr_unsetbit_mc() {
	local mac=$1

	printf "%02x:%s" $((0x${mac%%:*} & ~0x01)) ${mac#*:}
}

macaddr_random() {
	local randsrc=$(get_mac_binary /dev/urandom 0)
	
	echo "$(macaddr_unsetbit_mc "$(macaddr_setbit_la "${randsrc}")")"
}

macaddr_2bin() {
	local mac=$1

	echo -ne \\x${mac//:/\\x}
}

macaddr_canonicalize() {
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

dt_is_enabled() {
	grep -q okay "/proc/device-tree/$1/status"
}
