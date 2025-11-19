#!/bin/sh /etc/rc.common

START=99

boot() {
	case $(board_name) in
	seewo,srcm3588*)
		echo 1 > /sys/bus/pci/rescan
		;;
	esac
}
