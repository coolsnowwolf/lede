#!/bin/sh
#
# Copyright (C) 2018 rosysong@rosinson.com
#

. /lib/functions/network.sh
. /lib/nft-qos/core.sh

P1=""; P2=""; P3=""; P4=""; P5=""; P6="";
P7=""; P8=""; P9=""; P10=""; P11="";

qosdef_validate_priority() {
	uci_load_validate nft-qos default "$1" "$2" \
		'priority_enable:bool:0' \
		'priority_netdev:maxlength(8)'
}

_qosdef_handle_protox() { # <priority> <rule>
	case "$1" in
		-400) P1="$P1""$2";;
		-300) P2="$P2""$2";;
		-225) P3="$P3""$2";;
		-200) P4="$P4""$2";;
		-150) P5="$P5""$2";;
		-100) P6="$P6""$2";;
		0) P7="$P7""$2";;
		50) P8="$P8""$2";;
		100) P9="$P9""$2";;
		225) P10="$P10""$2";;
		300) P11="$P11""$2";;
	esac
}

qosdef_handle_protox() { # <section>
	local proto prio srv

	config_get proto $1 'protocol'
	config_get prio $1 'priority'
	config_get srv $1 'service'

	[ -z "$proto" -o \
		-z "$prio" -o \
		-z "$srv" ] && return

	_qosdef_handle_protox $prio \
	    "\t\t$proto dport { $srv } accept\n"
}

qosdef_append_rule_protox() { # <section>
	config_foreach qosdef_handle_protox $1
	qosdef_appendx \
		"${P1}${P2}${P3}${P4}${P5}${P6}${P7}${P8}${P9}${P10}${P11}"
}

qosdef_append_chain_priority() { # <name> <section> <device>
	local name=$1 device=$3

	qosdef_appendx "\tchain $name {\n"
	qosdef_append_chain_ingress filter $device 0 accept
	qosdef_append_rule_protox $2
	qosdef_appendx "\t}\n"
}

qosdef_remove_priority() {
	qosdef_remove_table netdev nft-qos-priority
}

# init traffic priority
qosdef_init_priority() {
	local ifname="br-lan"

	[ "$2" = 0 ] || {
		logger -t nft-qos-priority "validation failed"
		return 1
	}

	[ $priority_enable -eq 0 ] && return 1

	case "$priority_netdev" in
		lan) [ "$(uci_get network.lan.type)" != "bridge" ] && {
				network_get_device ifname "$priority_netdev" || \
				ifname="$(uci_get network.lan.ifname)"
			}
		;;
		wan*) network_get_device ifname "$priority_netdev" || \
			ifname="$(uci_get network.$priority_netdev.ifname)"
		;;
	esac

	qosdef_appendx "table netdev nft-qos-priority {\n"
	qosdef_append_chain_priority filter priority $ifname
	qosdef_appendx "}\n"
}
