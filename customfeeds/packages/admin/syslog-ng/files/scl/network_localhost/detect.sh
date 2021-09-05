#!/bin/sh
if [ "$(sysctl -n net.ipv6.conf.lo.disable_ipv6)" = "0" ]; then
	echo 'network(ip("::1") port(514) transport(udp) ip-protocol(6) )'
else
	echo 'network(ip("127.0.0.1") port(514) transport(udp) ip-protocol(4) )'
fi
