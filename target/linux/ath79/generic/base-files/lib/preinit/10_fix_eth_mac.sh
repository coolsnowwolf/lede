. /lib/functions.sh

preinit_set_mac_address() {
	case $(board_name) in
	avm,fritz1750e|\
	avm,fritz450e|\
	avm,fritzdvbc)
		ip link set dev eth0 address $(fritz_tffs -n maca -i $(find_mtd_part "tffs (1)"))
		;;
	dlink,dap-2695-a1)
		ip link set dev eth0 address $(mtd_get_mac_ascii bdcfg "lanmac")
		ip link set dev eth1 address $(mtd_get_mac_ascii bdcfg "wanmac")
		;;
	enterasys,ws-ap3705i)
		ip link set dev eth0 address $(mtd_get_mac_ascii u-boot-env0 ethaddr)
		;;
	siemens,ws-ap3610)
		ip link set dev eth0 address $(mtd_get_mac_ascii cfg1 ethaddr)
		;;
	esac
}

boot_hook_add preinit_main preinit_set_mac_address
