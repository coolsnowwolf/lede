. /lib/functions.sh

preinit_set_mac_address() {
	case $(board_name) in
	asrock,g10)
		lan_mac=$(mtd_get_mac_ascii hwconfig HW.LAN.MAC.Address)
		wan_mac=$(mtd_get_mac_ascii hwconfig HW.WAN.MAC.Address)
		ip link set dev eth0 address "${lan_mac}"
		ip link set dev eth1 address "${wan_mac}"
		;;
	esac
}

boot_hook_add preinit_main preinit_set_mac_address
