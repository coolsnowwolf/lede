#!/bin/sh

. /lib/functions.sh

if [ "$ACTION" != "ifup" ]; then
	exit
fi

config_load network

config_get teql $INTERFACE teql

if [ "$teql" != "" ]; then
    logger Adding device $DEVICE to TEQL master $teql
    insmod sch_teql
    tc qdisc add dev $DEVICE root $teql

    # The kernel doesn't let us bring it up until it has at least one
    # slave. So bring it up now, if it isn't already.
    if ! cat /sys/class/net/$teql/carrier &>/dev/null; then
        ifup $teql &
    fi
fi
