#!/bin/sh

dev=br-lan

stop_qos() {
	tc qdisc del dev $dev root 2>/dev/null
	tc qdisc del dev $dev ingress 2>/dev/null
	tc qdisc del dev ${dev}-ifb root 2>/dev/null
	ip link del dev ${dev}-ifb 2>/dev/null
}

start_qos() {
	local dl=$1
	local up=$2
	
	tc qdisc add dev $dev root handle 1: htb
	tc class add dev $dev parent 1: classid 1:1 htb rate ${dl}mbit
	
	ip link add dev ${dev}-ifb name ${dev}-ifb type ifb
	ip link set dev ${dev}-ifb up
	
	tc qdisc add dev ${dev}-ifb root handle 1: htb
	tc class add dev ${dev}-ifb parent 1: classid 1:1 htb rate ${up}mbit
	
	tc qdisc add dev $dev ingress
	tc filter add dev $dev parent ffff: protocol ip u32 match u32 0 0 flowid 1:1 action mirred egress redirect dev ${dev}-ifb
}

case "$1" in
	"stop")
		stop_qos
	;;
	"start")
		stop_qos
		start_qos $2 $3
	;;
	"add")
		ip="$2"
		dl="$3"
		up="$4"
		
		cnt=$(tc class show dev $dev | wc -l)
		
		tc class add dev $dev parent 1:1 classid 1:1$cnt htb rate ${dl}mbit ceil ${dl}mbit
		tc filter add dev $dev parent 1:0 protocol ip u32 match ip dst $ip flowid 1:1$cnt
		
		tc class add dev ${dev}-ifb parent 1:1 classid 1:1$cnt htb rate ${up}mbit ceil ${up}mbit
		tc filter add dev ${dev}-ifb parent 1:0 protocol ip u32 match ip src $ip flowid 1:1$cnt
	;;
	*)
		echo "Usage: $0 <command> [options]"
		echo "Commands:"
		echo "  start dl_rate up_rate       #Total bandwidth (Mbit/s)"
		echo "  stop"
		echo "  add ip dl_rate up_rate      #Limiting the bandwidth of a single IP (Mbit/s)"
		echo "Example:"
		echo "  $0 start 30 20              # Total bandwidth: down 30Mbit/s up 20Mbit/s"
		echo "  $0 add 192.168.22.12 10 2   # down 10Mbit/s  up 2Mbit/s"
	;;
esac