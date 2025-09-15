#!/bin/sh /etc/rc.common
#
# Activate Voice CPE TAPI subsystem LL driver for VMMC

START=31

start() {
	for i in 10 11 12 13 14 15 16 17 18; do
		if ! [ -e /dev/vmmc$i ]; then
			mknod -m 664 /dev/vmmc$i c 122 $i
		fi
	done
}
