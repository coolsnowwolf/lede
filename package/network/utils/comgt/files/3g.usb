#!/bin/sh
. /lib/functions.sh
. /lib/netifd/netifd-proto.sh

find_3g_iface() {
	local cfg="$1"
	local tty="$2"

	local proto
	config_get proto "$cfg" proto
	[ "$proto" = 3g ] || [ "$proto" = ncm ] || return 0

	# bypass state vars here because 00-netstate could clobber .device
	local dev=$(uci_get network "$cfg" device)

	if [ "${dev##*/}" = "${tty##*/}" ]; then
		if [ "$ACTION" = add ]; then
			proto_set_available "$cfg" 1
		fi
		if [ "$ACTION" = remove ]; then
			proto_set_available "$cfg" 0
		fi
	fi
}

[ "$ACTION" = add ] || [ "$ACTION" = remove ] || exit 0

case "$DEVICENAME" in
	tty*)
		[ -e "/dev/$DEVICENAME" ] || [ "$ACTION" = remove ] || exit 0
		config_load network
		config_foreach find_3g_iface interface "/dev/$DEVICENAME"
	;;
esac

