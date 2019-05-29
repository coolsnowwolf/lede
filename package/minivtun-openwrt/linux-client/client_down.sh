#!/bin/sh

# This script will be executed when client is down. All key value pairs (except
# password) in ShadowVPN config file will be passed to this script as
# environment variables.

# turn off NAT over VPN
iptables -t nat -D POSTROUTING -o $intf -j MASQUERADE
iptables -D FORWARD -i $intf -m state --state NEW,RELATED,ESTABLISHED -j ACCEPT
iptables -D FORWARD -o $intf -j ACCEPT

# Restore routing table
ip route del $server
ip route del   0/1
ip route del 128/1

# change to default DNS
mv /tmp/resolv.conf.ShadowVPN_original /etc/resolv.conf

# Remove route rules
if [ -f /tmp/minivtun_routes ]; then
	sed -e "s/^/route del /" /tmp/minivtun_routes | ip -batch -
	echo "Route rules have been removed"
	rm -rf /tmp/minivtun_routes
fi

echo $0 done