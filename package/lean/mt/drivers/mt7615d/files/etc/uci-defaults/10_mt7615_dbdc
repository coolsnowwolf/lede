#!/bin/sh

# fix isolate
sed -i 's/multicast_to_unicast:-1/multicast_to_unicast:-0/g' /lib/netifd/netifd-wireless.sh

# generate default wireless config
[ ! -f /etc/config/wireless ] && /sbin/wifi config

exit 0
