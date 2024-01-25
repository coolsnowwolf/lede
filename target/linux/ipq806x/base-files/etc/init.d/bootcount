#!/bin/sh /etc/rc.common

START=99

. "$IPKG_INSTROOT/lib/upgrade/asrock.sh"

boot() {
	case $(board_name) in
	asrock,g10)
		asrock_bootconfig_mangle "bootcheck" && reboot
		;;
	edgecore,ecw5410)
		fw_setenv bootcount 0
		;;
	linksys,ea7500-v1 |\
	linksys,ea8500)
		mtd resetbc s_env || true
		;;
	esac
}
