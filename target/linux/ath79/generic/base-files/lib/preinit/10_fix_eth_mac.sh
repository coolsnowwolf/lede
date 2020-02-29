#!/bin/sh

. /lib/functions.sh

preinit_set_mac_address() {
	case $(board_name) in
	siemens,ws-ap3610)
		ip link set dev eth0 address $(mtd_get_mac_ascii cfg1 ethaddr)
		;;
	esac
}

boot_hook_add preinit_main preinit_set_mac_address
