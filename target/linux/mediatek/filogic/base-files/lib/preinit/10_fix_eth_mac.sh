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
	esac
}

boot_hook_add preinit_main preinit_set_mac_address
