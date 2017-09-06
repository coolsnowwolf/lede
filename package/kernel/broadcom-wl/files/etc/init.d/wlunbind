#!/bin/sh /etc/rc.common
# Copyright (C) 2010-2011 OpenWrt.org

START=09

unbind_driver() {
	local driver="$1"
	local sysfs="/sys/bus/pci/drivers/$driver"
	if [ -d "$sysfs" ]; then
		local lnk
		for lnk in $sysfs/*; do
			[ -h "$lnk" ] || continue
			case "${lnk##*/}" in
				*:*:*.*)
					logger "Unbinding WL PCI device ${lnk##*/} from $driver"
					echo -n "${lnk##*/}" > "$sysfs/unbind"
				;;
			esac
		done
	fi
}

boot() {
	unbind_driver b43-pci-bridge
	unbind_driver bcma-pci-bridge
}

start() { :; }
stop() { :; }
