. /lib/functions.sh

preinit_set_mac_address() {
	case $(board_name) in
	asus,map-ac2200)
		base_mac=$(mtd_get_mac_binary_ubi Factory 0x1006)
		ip link set dev eth0 address $(macaddr_add "$base_mac" 1)
		ip link set dev eth1 address $(macaddr_add "$base_mac" 3)
		;;
	asus,rt-ac42u)
		ip link set dev eth0 address $(mtd_get_mac_binary_ubi Factory 0x1006)
		ip link set dev eth1 address $(mtd_get_mac_binary_ubi Factory 0x9006)
		;;
	engenius,eap2200)
		base_mac=$(cat /sys/class/net/eth0/address)
		ip link set dev eth1 address $(macaddr_add "$base_mac" 1)
		;;
	linksys,ea8300|\
	linksys,mr8300)
		base_mac=$(mtd_get_mac_ascii devinfo hw_mac_addr)
		ip link set dev eth0 address "$base_mac"
		ip link set dev eth1 address $(macaddr_add "$base_mac" 1)
		;;
	meraki,mr33)
		mac_lan=$(get_mac_binary "/sys/bus/i2c/devices/0-0050/eeprom" 0x66)
		[ -n "$mac_lan" ] && ip link set dev eth0 address "$mac_lan"
		;;
	zyxel,nbg6617)
		base_mac=$(cat /sys/class/net/eth0/address)
		ip link set dev eth0 address $(macaddr_add "$base_mac" 2)
		ip link set dev eth1 address $(macaddr_add "$base_mac" 3)
		;;
	esac
}

boot_hook_add preinit_main preinit_set_mac_address
