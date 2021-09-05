#!/bin/sh

NAME=ap51-flash
rootfs=""
kernel=""
ubnt=""

[ "$1" -eq "1" ] && loop="1"
ifname="$2"
[ -n "$3" ] && rootfs="--rootfs $3"
[ -n "$4" ] && kernel="--kernel $4"
[ -n "$5" ] && ubnt="--ubnt $5"

while true; do
	start-stop-daemon -S -m -p "/var/run/$NAME-$ifname.pid" -n $NAME \
		-x /usr/sbin/$NAME -- --flash-from-file "$rootfs" "$kernel" "$ubnt" "$ifname"

	rm "/var/run/$NAME-$ifname.pid"
	[ "$loop" != "1" ] && break
	sleep 15
done
