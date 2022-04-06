. /lib/functions.sh
. /lib/functions/system.sh

preinit_set_mac_address() {
	case $(board_name) in
	zyxel,emg2926-q10a|\
	zyxel,nbg6716)
		ethaddr=$(mtd_get_mac_ascii u-boot-env ethaddr)
		ip link set dev eth0 address $(macaddr_add $ethaddr 2)
		ip link set dev eth1 address $(macaddr_add $ethaddr 3)
		;;
	esac
}

boot_hook_add preinit_main preinit_set_mac_address
