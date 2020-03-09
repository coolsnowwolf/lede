#!/bin/sh /etc/rc.common

START=99

boot() {
	case $(board_name) in
	alfa-network,ac1200rm|\
	alfa-network,r36m-e4g|\
	alfa-network,tube-e4g)
		[ -n "$(fw_printenv bootcount bootchanged 2>/dev/null)" ] &&\
			echo -e "bootcount\nbootchanged\n" | /usr/sbin/fw_setenv -s -
		;;
	esac
}
