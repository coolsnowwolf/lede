#!/bin/sh /etc/rc.common

START=99

boot() {
	case $(board_name) in
	linksys,mr7350)
		mtd resetbc s_env || true
	;;
	esac
}
