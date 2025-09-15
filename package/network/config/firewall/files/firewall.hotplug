#!/bin/sh

[ "$ACTION" = ifup -o "$ACTION" = ifupdate ] || exit 0
[ "$ACTION" = ifupdate -a -z "$IFUPDATE_ADDRESSES" -a -z "$IFUPDATE_DATA" ] && exit 0

/etc/init.d/firewall enabled || exit 0

fw3 -q network "$INTERFACE" >/dev/null || exit 0

logger -t firewall "Reloading firewall due to $ACTION of $INTERFACE ($DEVICE)"
fw3 -q reload
