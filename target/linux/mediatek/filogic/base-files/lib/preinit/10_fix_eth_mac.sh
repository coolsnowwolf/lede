. /lib/functions/system.sh

preinit_set_mac_address() {
	case $(board_name) in
	asus,tuf-ax4200|\
	asus,tuf-ax6000)
		CI_UBIPART="UBI_DEV"
		addr=$(mtd_get_mac_binary_ubi "Factory" 0x4)
		ip link set dev eth0 address "$addr"
		ip link set dev eth1 address "$addr"
		;;
 	nradio,c8-668gl|\
	nradio,c8-660)
		lan_mac=$(mmc_get_mac_ascii bdinfo "fac_mac ")
		test -n "$lan_mac" || lan_mac=$(mtd_get_mac_ascii bdinfo "fac_mac ")
		wan_mac=$(macaddr_add "$lan_mac" -1)
		ip link set dev eth0 address "$lan_mac"
		ip link set dev eth1 address "$wan_mac"
		;;
	*) 
	esac
}

boot_hook_add preinit_main preinit_set_mac_address
