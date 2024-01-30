#!/bin/sh /etc/rc.common

START=99

boot() {
	case $(board_name) in
	edgecore,eap102)
		fw_setenv upgrade_available 0
		# Unset changed flag after sysupgrade complete
		fw_setenv changed
	;;
	esac
}
