#!/bin/sh

. /lib/functions.sh

preinit_set_mac_address() {
	case $(board_name) in
	ruijie,rg-mtfi-m520)
        base_mac=$(mtd_get_mac_ascii PRODUCTINFO ethaddr)
        ip link set dev eth0 address $(macaddr_add "$base_mac" +1)
        ip link set dev eth1 address $(macaddr_add "$base_mac" +2)
        ;;
	esac
}

boot_hook_add preinit_main preinit_set_mac_address
