#!/bin/sh

. /lib/functions.sh

preinit_set_mac_address() {
	case $(board_name) in
	asus,map-ac2200)
		base_mac=$(mtd_get_mac_binary_ubi Factory 4102)
		ip link set dev eth0 address $(macaddr_add "$base_mac" +1)
		ip link set dev eth1 address $(macaddr_add "$base_mac" +3)
		;;
	asus,rt-acrh17|\
	asus,rt-ac58u)
		CI_UBIPART=UBI_DEV
		mac=$(macaddr_add $(mtd_get_mac_binary_ubi Factory 4102) +1)
		ifconfig eth0 hw ether $mac 2>/dev/null
		;;
	meraki,mr33)
		mac_lan=$(get_mac_binary "/sys/bus/i2c/devices/0-0050/eeprom" 102)
		[ -n "$mac_lan" ] && ip link set dev eth0 address "$mac_lan"
		;;
	zyxel,nbg6617)
		base_mac=$(cat /sys/class/net/eth0/address)
		ip link set dev eth0 address $(macaddr_add "$base_mac" +2)
		;;
	esac
}

boot_hook_add preinit_main preinit_set_mac_address
