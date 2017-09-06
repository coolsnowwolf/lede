#!/bin/sh

if [ "$ACTION" = "pressed" -a "$BUTTON" = "wps" ]; then
	cd /var/run/hostapd
	for socket in *; do
		[ -S "$socket" ] || continue
		hostapd_cli -i "$socket" wps_pbc
	done
fi

return 0
