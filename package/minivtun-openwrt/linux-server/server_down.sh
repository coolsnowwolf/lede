#!/bin/sh
# Created by clowwindy
#
# This script will be executed when server is down. All key value pairs (except
# password) in ShadowVPN config file will be passed to this script as
# environment variables.

# Turn off NAT over VPN
iptables -t nat -D POSTROUTING -s $local_addr ! -d $local_addr -m comment --comment "minivtun" -j MASQUERADE
iptables -D FORWARD -s $local_addr -m state --state NEW,RELATED,ESTABLISHED -j ACCEPT
iptables -D FORWARD -d $local_addr -j ACCEPT

# Turn off MSS fix (MSS = MTU - TCP header - IP header)
iptables -t mangle -D FORWARD -p tcp -m tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu

echo $0 done
