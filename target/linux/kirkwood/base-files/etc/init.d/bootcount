#!/bin/sh /etc/rc.common

START=99

boot() {
	case $(board_name) in
	linksys,e4200-v2|\
	linksys,ea3500|\
	linksys,ea4500)
		mtd resetbc s_env || true
		;;
	esac
}
