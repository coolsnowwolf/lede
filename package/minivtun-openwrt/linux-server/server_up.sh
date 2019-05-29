#!/bin/sh
# Created by clowwindy
#
# This script will be executed when server is down. All key value pairs (except
# password) in ShadowVPN config file will be passed to this script as
# environment variables.

# Turn on IP forwarding
sysctl -w net.ipv4.ip_forward=1

# turn on NAT over VPN
if !(iptables-save -t nat | grep -q "minivtun"); then
  iptables -t nat -A POSTROUTING -s $local_addr ! -d $local_addr -m comment --comment "minivtun" -j MASQUERADE
fi
iptables -A FORWARD -s $local_addr -m state --state NEW,RELATED,ESTABLISHED -j ACCEPT
iptables -A FORWARD -d $local_addr -j ACCEPT

# Turn on MSS fix (MSS = MTU - TCP header - IP header)
iptables -t mangle -A FORWARD -p tcp -m tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu

echo $0 done
