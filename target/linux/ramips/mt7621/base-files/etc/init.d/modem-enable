#!/bin/sh /etc/rc.common

START=99

boot() {
	case $(board_name) in
	huasifei,mt7621dtu)
		echo 1 > /sys/class/gpio/modem_reset/value
		sleep 5
		sendat 0 AT+GTUSBMODE=32
		sendat 0 AT+CGDCONT=2,"IP","CMNET"
		sendat 0 AT+GTRNDIS=1,2
		;;
	esac
}
