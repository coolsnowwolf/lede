#!/bin/sh
#
# Copyright (C) 2018 OpenWrt.org
# Copyright (C) 2018 rosysong@rosinson.com
#

NFT_QOS_SCRIPT_TEXT=
NFT_QOS_SCRIPT_FILE=/tmp/qos.nft

nft_qos_appendx() { # <string to be appended>
	NFT_QOS_SCRIPT_TEXT="$NFT_QOS_SCRIPT_TEXT""$1"
}

nft_qos_append_header() { # add header for nft script
	nft_qos_appendx "#!/usr/sbin/nft -f\n"
	nft_qos_appendx "# Copyright (C) 2018 rosysong@rosinson.com\n"
	nft_qos_appendx "#\n\n"
}

# nft_qos_append_{MATCH}_{STATEMENT}
nft_qos_append_ether_limit() { # <section> <mac> <operator>
	local mac=$2 rate unit \
		operator=$3 policy=drop

	[ -z "$mac" ] && return

	config_get rate $1 rate 512
	config_get unit $1 unit kbytes

	nft_qos_appendx \
	    "\t\tether $operator $mac limit rate over $rate $unit/second $policy\n"
}

# nft_qos_append_{MATCH}_{STATEMENT}
nft_qos_append_ip_limit() { # <section> <ipaddr> <operator>
	local ipaddr=$2 rate unit \
		operator=$3 policy=drop

	config_get rate $1 rate 512
	config_get unit $1 unit kbytes

	[ -z "$ipaddr" ] && return

	nft_qos_appendx \
	    "\t\tip $operator $ipaddr limit rate over $rate $unit/second $policy\n"
}

nft_qos_append_rule() { # <section> <chain>
	local ipaddr macaddr operator

	config_get ipaddr $1 ipaddr
	config_get macaddr $1 macaddr

	[ -z "$ipaddr" -a -z "$macaddr" ] && return

	case "$2" in
		download) operator=daddr;;
		upload) operator=saddr;;
	esac

	if [ -n "$macaddr" -a ! "$2" = "input" ]; then
		nft_qos_append_ether_limit $1 $macaddr $operator
	else
		nft_qos_append_ip_limit $1 $ipaddr $operator
	fi
}

nft_qos_append_chain_def() { # <type> <hook> <priority> <policy>
	nft_qos_appendx "\t\ttype $1 hook $2 priority $3; policy $4;\n"
}

nft_qos_append_chain() { # <hook> <name> <section>
	local ctype=filter hook=$1 priority=0 \
		policy=accept name=$2

	nft_qos_appendx "\tchain $name {\n"
	nft_qos_append_chain_def $ctype $hook $priority $policy
	config_foreach nft_qos_append_rule $2 $3
	nft_qos_appendx "\t}\n"
}

nft_qos_append_commands() {
	local en

	en=$(uci get nft-qos.@default[0].enabled 2>/dev/null)

	[ $en -eq 0 ] && return

	nft_qos_appendx "table ip qos {\n"
	nft_qos_append_chain input upload upload
	nft_qos_append_chain postrouting download download
	nft_qos_appendx "}\n"
}

nft_qos_flush() { # <name of table>
	/usr/sbin/nft flush table $1 2>/dev/null
}

nft_qos_init() {
	echo -e $NFT_QOS_SCRIPT_TEXT > $NFT_QOS_SCRIPT_FILE
}

nft_qos_start() {
	/usr/sbin/nft -f $NFT_QOS_SCRIPT_FILE
}
