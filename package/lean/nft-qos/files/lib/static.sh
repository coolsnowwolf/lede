#!/bin/sh
#
# Copyright (C) 2018 rosysong@rosinson.com
#

. /lib/nft-qos/core.sh

# append rule for static qos
qosdef_append_rule_sta() { # <section> <operator> <default-unit> <default-rate>
	local ipaddr unit rate
	local operator=$2

	config_get ipaddr $1 ipaddr
	config_get unit $1 unit $3
	config_get rate $1 rate $4

	[ -z "$ipaddr" ] && return

	qosdef_append_rule_ip_limit $ipaddr $operator $unit $rate
}

# append chain for static qos
qosdef_append_chain_sta() { # <hook> <name> <section> <unit> <rate>
	local hook=$1 name=$2
	local config=$3 operator

	case "$name" in
		download) operator="daddr";;
		upload) operator="saddr";;
	esac

	qosdef_appendx "\tchain $name {\n"
	qosdef_append_chain_def filter $hook 0 accept
	qosdef_append_rule_limit_whitelist $name
	config_foreach qosdef_append_rule_sta $config $operator $4 $5
	qosdef_appendx "\t}\n"
}

qosdef_flush_static() {
	qosdef_flush_table "$NFT_QOS_INET_FAMILY" nft-qos-static
}

# static limit rate init
qosdef_init_static() {
	local unit_dl unit_ul rate_dl rate_ul
	local limit_enable limit_type hook_ul="prerouting" hook_dl="postrouting"

	uci_validate_section nft-qos default default \
		'limit_enable:bool:0' \
		'limit_type:maxlength(8)' \
		'static_unit_dl:string:kbytes' \
		'static_unit_ul:string:kbytes' \
		'static_rate_dl:uinteger:50' \
		'static_rate_ul:uinteger:50'

	[ $? -ne 0 ] && {
		logger -t nft-qos-static "validation failed"
		return 1
	}

	[ $limit_enable -eq 0 -o \
		$limit_type = "dynamic" ] && return 1

	[ -z "$NFT_QOS_HAS_BRIDGE" ] && {
		hook_ul="postrouting"
		hook_dl="prerouting"
	}

	qosdef_appendx "table $NFT_QOS_INET_FAMILY nft-qos-static {\n"
	qosdef_append_chain_sta $hook_ul upload upload $unit_ul $rate_ul
	qosdef_append_chain_sta $hook_dl download download $unit_dl $rate_dl
	qosdef_appendx "}\n"
}
