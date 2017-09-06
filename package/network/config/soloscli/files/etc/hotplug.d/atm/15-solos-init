#!/bin/sh

dialog() {
	local tag="$(echo "$1" | cut -d= -f1)"
	local value="$(echo "$1" | cut -d= -f2-)"
	local response
	
	response="$(soloscli -s "$port" "$tag" "$value")"
	[ $? -ne 0 ] && {
		logger "soloscli($port): $tag '$value' returns $response"
	}
}

if [ "$ACTION" = "add" ]; then
	include /lib/network
	scan_interfaces

	case $DEVICENAME in
	solos-pci[0-3])
		port="${DEVICENAME#solos-pci}"
		device="solos${port}"

		config_list_foreach wan "$device" dialog
		;;
	esac
fi
