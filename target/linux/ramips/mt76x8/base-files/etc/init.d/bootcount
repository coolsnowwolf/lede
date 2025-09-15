#!/bin/sh /etc/rc.common

START=99

boot() {
	case $(board_name) in
	alfa-network,awusfree1)
		[ -n "$(fw_printenv bootcount bootchanged 2>/dev/null)" ] &&\
			echo -e "bootcount\nbootchanged\n" | /usr/sbin/fw_setenv -s -
		;;
	xiaomi,mi-router-4c|\
	xiaomi,miwifi-nano)
		fw_setenv flag_boot_success 1
		;;
	esac
}
