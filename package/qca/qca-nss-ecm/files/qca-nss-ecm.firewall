#!/bin/sh
if [ ! -r /sbin/fw4 ]; then
iptables-save|grep physdev-is-bridged|while read a; do
	iptables -D FORWARD -m physdev --physdev-is-bridged -j ACCEPT
done
iptables -I FORWARD 1 -m physdev --physdev-is-bridged -j ACCEPT
ip6tables-save|grep physdev-is-bridged|while read a; do
	ip6tables -D FORWARD -m physdev --physdev-is-bridged -j ACCEPT
done
ip6tables -I FORWARD 1 -m physdev --physdev-is-bridged -j ACCEPT
fi
