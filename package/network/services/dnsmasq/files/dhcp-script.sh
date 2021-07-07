#!/bin/sh

[ -f "$USER_DHCPSCRIPT" ] && . "$USER_DHCPSCRIPT" "$@"

. /usr/share/libubox/jshn.sh

json_init
json_add_array env
hotplugobj=""

case "$1" in
	add | del | old | arp-add | arp-del)
		json_add_string "" "MACADDR=$2"
		json_add_string "" "IPADDR=$3"
	;;
esac

case "$1" in
	add)
		json_add_string "" "ACTION=add"
		json_add_string "" "HOSTNAME=$4"
		hotplugobj="dhcp"
	;;
	del)
		json_add_string "" "ACTION=remove"
		json_add_string "" "HOSTNAME=$4"
		hotplugobj="dhcp"
	;;
	old)
		json_add_string "" "ACTION=update"
		json_add_string "" "HOSTNAME=$4"
		hotplugobj="dhcp"
	;;
	arp-add)
		json_add_string "" "ACTION=add"
		hotplugobj="neigh"
	;;
	arp-del)
		json_add_string "" "ACTION=remove"
		hotplugobj="neigh"
	;;
	tftp)
		json_add_string "" "ACTION=add"
		json_add_string "" "TFTP_SIZE=$2"
		json_add_string "" "TFTP_ADDR=$3"
		json_add_string "" "TFTP_PATH=$4"
		hotplugobj="tftp"
	;;
esac

json_close_array env

[ -n "$hotplugobj" ] && ubus call hotplug.${hotplugobj} call "$(json_dump)"
