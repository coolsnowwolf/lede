#!/bin/sh
. /lib/functions/network.sh
device="$1"; shift
network_defer_device "$device"
exec /usr/sbin/br2684ctl "$@"
