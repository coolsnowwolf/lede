#!/bin/sh

for iface in $(tc qdisc show | grep -E '(hfsc|ingress)' | awk '{print $5}'); do
	tc qdisc del dev "$iface" ingress 2>&- >&-
	tc qdisc del dev "$iface" root 2>&- >&-
done
/usr/lib/qos/generate.sh firewall stop | sh
