#!/bin/sh

[ -f "$USER_DHCPSCRIPT" ] && . "$USER_DHCPSCRIPT" "$@"

case "$1" in
	add)
		export ACTION="add"
		export MACADDR="$2"
		export IPADDR="$3"
		export HOSTNAME="$4"
		exec /sbin/hotplug-call dhcp
	;;
	del)
		export ACTION="remove"
		export MACADDR="$2"
		export IPADDR="$3"
		export HOSTNAME="$4"
		exec /sbin/hotplug-call dhcp
	;;
	old)
		export ACTION="update"
		export MACADDR="$2"
		export IPADDR="$3"
		export HOSTNAME="$4"
		exec /sbin/hotplug-call dhcp
	;;
	arp-add)
		export ACTION="add"
		export MACADDR="$2"
		export IPADDR="$3"
		exec /sbin/hotplug-call neigh
	;;
	arp-del)
		export ACTION="remove"
		export MACADDR="$2"
		export IPADDR="$3"
		exec /sbin/hotplug-call neigh
	;;
	tftp)
		export ACTION="add"
		export TFTP_SIZE="$2"
		export TFTP_ADDR="$3"
		export TFTP_PATH="$4"
		exec /sbin/hotplug-call tftp
	;;
esac
