#!/bin/sh
# Busybox udhcpc dispatcher script. Copyright (C) 2009 by Axel Beckert.
#
# Based on the busybox example scripts and the old udhcp source
# package default.* scripts.

RESOLV_CONF="/etc/resolv.conf"

case $1 in
    bound|renew)
	[ -n "$broadcast" ] && BROADCAST="broadcast $broadcast"
	[ -n "$subnet" ] && NETMASK="netmask $subnet"

	/sbin/ifconfig $interface $ip $BROADCAST $NETMASK

	if [ -n "$router" ]; then
	    echo "$0: Resetting default routes"
	    while /sbin/route del default gw 0.0.0.0 dev $interface; do :; done

	    metric=0
	    for i in $router; do
		/sbin/route add default gw $i dev $interface metric $metric
		metric=$(($metric + 1))
	    done
	fi

	# Update resolver configuration file
	R=""
	[ -n "$domain" ] && R="domain $domain
"
	for i in $dns; do
	    echo "$0: Adding DNS $i"
	    R="${R}nameserver $i
"
	done

	if [ -x /sbin/resolvconf ]; then
	    echo -n "$R" | resolvconf -a "${interface}.udhcpc"
	else
	    echo -n "$R" > "$RESOLV_CONF"
	fi
	;;

    deconfig)
	if [ -x /sbin/resolvconf ]; then
	    resolvconf -d "${interface}.udhcpc"
	fi
	/sbin/ifconfig $interface 0.0.0.0
	;;

    leasefail)
	echo "$0: Lease failed: $message"
	;;

    nak)
	echo "$0: Received a NAK: $message"
	;;

    *)
	echo "$0: Unknown udhcpc command: $1";
	exit 1;
	;;
esac
