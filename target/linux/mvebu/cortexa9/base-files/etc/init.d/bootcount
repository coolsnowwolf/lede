#!/bin/sh /etc/rc.common

START=99

boot() {
	case $(board_name) in
	linksys,wrt1200ac|\
	linksys,wrt1900ac-v1|\
	linksys,wrt1900ac-v2|\
	linksys,wrt1900acs|\
	linksys,wrt3200acm|\
	linksys,wrt32x)
		mtd resetbc s_env || true
		;;
	esac
}
