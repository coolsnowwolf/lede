#!/bin/sh
#
# Copyright (C) 2018 rosysong@rosinson.com
#

# for uci_validate_section()
. /lib/functions/procd.sh

NFT_QOS_HAS_BRIDGE=
NFT_QOS_INET_FAMILY=ip
NFT_QOS_SCRIPT_TEXT=
NFT_QOS_SCRIPT_FILE=/tmp/qos.nft

qosdef_appendx() { # <string to be appended>
	NFT_QOS_SCRIPT_TEXT="$NFT_QOS_SCRIPT_TEXT""$1"
}

qosdef_append_chain_def() { # <type> <hook> <priority> <policy>
	qosdef_appendx "\t\ttype $1 hook $2 priority $3; policy $4;\n"
}

qosdef_append_chain_ingress() { # <type> <device> <priority> <policy>
	qosdef_appendx "\t\ttype $1 hook ingress device $2 priority $3; policy $4;\n"
}

# qosdef_append_rule_{MATCH}_{STATEMENT}
qosdef_append_rule_ip_limit() { # <ipaddr> <operator> <unit> <rate>
	local ipaddr=$1
	local operator=$2
	local unit=$3
	local rate=$4

	qosdef_appendx \
	    "\t\tip $operator $ipaddr limit rate over $rate $unit/second drop\n"
}

# qosdef_append_rule_{MATCH}_{POLICY}
qosdef_append_rule_ip_policy() { # <operator> <ipaddr> <policy>
	qosdef_appendx "\t\tip $1 $2 $3\n"
}

_handle_limit_whitelist() { # <value> <chain>
	local ipaddr=$1
	local operator

	[ -z "$ipaddr" ] && return

	case "$2" in
		download) operator="daddr";;
		upload) operator="saddr";;
	esac

	qosdef_append_rule_ip_policy $operator $ipaddr accept
}

qosdef_append_rule_limit_whitelist() { # <chain>
	config_list_foreach default limit_whitelist _handle_limit_whitelist $1
}

qosdef_flush_table() { # <family> <table>
	nft flush table $1 $2 2>/dev/null
}

qosdef_remove_table() { # <family> <table>
	nft delete table $1 $2 2>/dev/null
}

qosdef_init_header() { # add header for nft script
	qosdef_appendx "#!/usr/sbin/nft -f\n"
	qosdef_appendx "# Copyright (C) 2018 rosysong@rosinson.com\n"
	qosdef_appendx "#\n\n"
}

qosdef_init_env() {
	# check interface type of lan
	local lt="$(uci_get "network.lan.type")"
	[ "$lt" = "bridge" ] && export NFT_QOS_HAS_BRIDGE="y"

	# check if ipv6 support
	[ -e /proc/sys/net/ipv6 ] && export NFT_QOS_INET_FAMILY="inet"
}

qosdef_clean_cache() {
	rm -f $NFT_QOS_SCRIPT_FILE
}

qosdef_init_done() {
	echo -e $NFT_QOS_SCRIPT_TEXT > $NFT_QOS_SCRIPT_FILE 2>/dev/null
}

qosdef_start() {
	nft -f $NFT_QOS_SCRIPT_FILE 2>/dev/null
}
