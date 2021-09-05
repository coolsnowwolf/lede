#!/bin/sh
# based on static.sh
# Copyright (C) 2020 Tong Zhang<ztong0001@gmail.com>
#

. /lib/nft-qos/core.sh

qosdef_validate_mac() {
	uci_load_validate nft-qos default "$1" "$2" \
		'limit_mac_enable:bool:0'
}

# append rule for mac qos
qosdef_append_rule_mac() { # <section> <operator>
	local macaddr unit rate
	local operator=$2

	config_get macaddr $1 macaddr
	if [ "$operator" = "saddr" ]; then
		config_get unit $1 urunit
		config_get rate $1 urate
	else
		config_get unit $1 drunit
		config_get rate $1 drate
	fi

	[ -z "$macaddr" ] && return

	qosdef_append_rule_mac_limit $macaddr $operator $unit $rate
}

# append chain for mac qos
qosdef_append_chain_mac() { # <hook> <name> <section>
	local hook=$1 name=$2
	local config=$3 operator

	case "$name" in
		download) operator="daddr";;
		upload) operator="saddr";;
	esac

	qosdef_appendx "\tchain $name {\n"
	qosdef_append_chain_def filter $hook 0 accept
	config_foreach qosdef_append_rule_mac $config $operator
	qosdef_appendx "\t}\n"
}

qosdef_flush_mac() {
	if [ -n "$NFT_QOS_HAS_BRIDGE" ]; then
		qosdef_flush_table bridge nft-qos-mac
	else
		qosdef_flush_table "$NFT_QOS_INET_FAMILY" nft-qos-mac
	fi
}

# limit rate by mac address init
qosdef_init_mac() {
	local hook_ul="prerouting" hook_dl="postrouting"

	[ "$2" = 0 ] || {
		logger -t nft-qos-mac "validation failed"
		return 1
	}

	[ $limit_mac_enable -eq 0 ] && return 1

	table_name=$NFT_QOS_INET_FAMILY
	if [ -z "$NFT_QOS_HAS_BRIDGE" ]; then
		hook_ul="postrouting"
		hook_dl="prerouting"
	else
		table_name="bridge"
	fi

	qosdef_appendx "table $table_name nft-qos-mac {\n"
	qosdef_append_chain_mac $hook_ul upload client
	qosdef_append_chain_mac $hook_dl download client
	qosdef_appendx "}\n"
}
