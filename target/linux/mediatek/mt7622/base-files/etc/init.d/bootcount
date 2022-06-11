#!/bin/sh /etc/rc.common

START=99

boot() {
	case $(board_name) in
	linksys,e8450)
		mtd erase senv || true
		;;
	esac
}
