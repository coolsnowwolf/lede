#!/bin/sh

LOCK_FILE=/var/lock/opkg.lock

sleep 5s
while [ -f "$LOCK_FILE" ]
do
	sleep 5s
done

opkg install /usr/share/smartdns-install/smartdns.ipk --force-depends
opkg install /usr/share/smartdns-install/luci-app-smartdns.ipk --force-depends

rm -rf /usr/share/smartdns-install/smartdns.ipk
rm -rf /usr/share/smartdns-install/luci-app-smartdns.ipk
opkg remove smartdns-install
rm -f /tmp/luci-indexcache

exit 0
