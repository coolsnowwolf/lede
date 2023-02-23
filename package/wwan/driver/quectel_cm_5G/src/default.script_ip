#!/bin/sh
# Busybox udhcpc dispatcher script. Copyright (C) 2009 by Axel Beckert.
#
# Based on the busybox example scripts and the old udhcp source
# package default.* scripts.

RESOLV_CONF="/etc/resolv.conf"
IPCMD=`which ip`

case $1 in
    bound|renew)
	$IPCMD address add broadcast $broadcast $ip/$subnet dev $interface

	if [ -n "$router" ]; then
	    echo "$0: Resetting default routes"
	    while $IPCMD route del default dev $interface; do :; done

	    metric=0
	    for i in $router; do
		$IPCMD route add default dev $interface via $router metric $metric
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
	$IPCMD address flush dev $interface
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
