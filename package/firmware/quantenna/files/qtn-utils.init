#!/bin/sh /etc/rc.common

NAME=qtn-utils
# must start before networking
START=19

USE_PROCD=1

start_service() {
	ifconfig host0 1.1.1.1 netmask 255.255.255.252 up
	local ifname="host0"
	local ipaddr="1.1.1.1"
	local hex_ip=$(printf "0x%02x%02x%02x%02x" $(echo $ipaddr | tr . \ ))
	
	eval "$(ipcalc.sh $ipaddr 255.255.255.252 2)"
	echo "1.1.1.2" >  /tmp/qcsapi_target_ip.conf    

	# work around Quatenna vlan bug - pop bogus tags from arp and ip packets for 1.1.1.1
	tc qdisc add dev $ifname handle ffff: ingress
	tc filter add dev $ifname parent ffff: pref 1 \
	   protocol 802.1q u32 \
	   match u16 0x0806 0xffff at -2 \
	   match u32 $hex_ip 0xffffffff at 24 \
	   action vlan pop 
	tc filter add dev $ifname parent ffff: pref 2 \
	   protocol 802.1q u32 \
	   match u16 0x0800 0xffff at -2 \
	   match u32 $hex_ip 0xffffffff at 16 \
	   action vlan pop
}

stop_service() {
	local ifname="host0"

	tc filter del dev $ifname ingress
	tc qdisc del dev $ifname ingress

}
