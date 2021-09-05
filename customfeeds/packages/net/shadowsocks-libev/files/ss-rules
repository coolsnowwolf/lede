#!/bin/sh -e
#
# Copyright (C) 2017 Yousong Zhou <yszhou4tech@gmail.com>
#
# The design idea was derived from ss-rules by Jian Chang <aa65535@live.com>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

__errmsg() {
	echo "ss-rules: $*" >&2
}

if [ "$1" = "-6" ]; then
	if ! ip6tables -t nat -L -n &>/dev/null; then
		__errmsg "Skipping ipv6.  Requires ip6tables-mod-nat"
		exit 1
	fi
	o_use_ipv6=1; shift
fi

ss_rules_usage() {
	cat >&2 <<EOF
Usage: ss-rules [options]

	-6              Operate on address family IPv6
	                When present, must be the first argument
	-h, --help      Show this help message then exit
	-f, --flush     Flush rules, ipset then exit
	-l <port>       Local port number of ss-redir with TCP mode
	-L <port>       Local port number of ss-redir with UDP mode
	-s <ips>        List of ip addresses of remote shadowsocks server
	--ifnames       Only apply rules on packets from these ifnames
	--src-bypass <ips|cidr>
	--src-forward <ips|cidr>
	--src-checkdst <ips|cidr>
	--src-default <bypass|forward|checkdst>
	                Packets will have their src ip checked in order against
	                bypass, forward, checkdst list and will bypass, forward
	                through, or continue to have their dst ip checked
	                respectively on the first match.  Otherwise, --src-default
	                decide the default action
	--dst-bypass <ips|cidr>
	--dst-forward <ips|cidr>
	--dst-bypass-file <file>
	--dst-forward-file <file>
	--dst-default <bypass|forward>
	                Same as with their --src-xx equivalent
	--dst-forward-recentrst
	                Forward those packets whose destinations have recently
	                sent to us multiple tcp-rst packets
	--local-default <bypass|forward|checkdst>
	                Default action for local out TCP traffic

The following ipsets will be created by ss-rules.  They are also intended to be
populated by other programs like dnsmasq with ipset support

	ss_rules_src_bypass
	ss_rules_src_forward
	ss_rules_src_checkdst
	ss_rules_dst_bypass
	ss_rules_dst_forward
EOF
}

o_dst_bypass4_="
	0.0.0.0/8
	10.0.0.0/8
	100.64.0.0/10
	127.0.0.0/8
	169.254.0.0/16
	172.16.0.0/12
	192.0.0.0/24
	192.0.2.0/24
	192.31.196.0/24
	192.52.193.0/24
	192.88.99.0/24
	192.168.0.0/16
	192.175.48.0/24
	198.18.0.0/15
	198.51.100.0/24
	203.0.113.0/24
	224.0.0.0/4
	240.0.0.0/4
	255.255.255.255
"
o_dst_bypass6_="
	::1/128
	::/128
	::ffff:0:0/96
	64:ff9b:1::/48
	100::/64
	2001:2::/48
	2001:db8::/32
	fe80::/10
	2001::/23
	fc00::/7
"
o_src_default=bypass
o_dst_default=bypass
o_local_default=bypass

alias grep_af="sed -ne '/:/!p'"
o_dst_bypass_="$o_dst_bypass4_"
if [ -n "$o_use_ipv6" ]; then
	alias grep_af="sed -ne /:/p"
	alias iptables=ip6tables
	alias iptables-save=ip6tables-save
	alias iptables-restore=ip6tables-restore
	alias ip="ip -6"
	o_af=6
	o_dst_bypass_="$o_dst_bypass6_"
fi

ss_rules_parse_args() {
	while [ "$#" -gt 0 ]; do
		case "$1" in
			-h|--help) ss_rules_usage; exit 0;;
			-f|--flush) ss_rules_flush; exit 0;;
			-l) o_redir_tcp_port="$2"; shift 2;;
			-L) o_redir_udp_port="$2"; shift 2;;
			-s) o_remote_servers="$2"; shift 2;;
			--ifnames) o_ifnames="$2"; shift 2;;
			--ipt-extra) o_ipt_extra="$2"; shift 2;;
			--src-default) o_src_default="$2"; shift 2;;
			--dst-default) o_dst_default="$2"; shift 2;;
			--local-default) o_local_default="$2"; shift 2;;
			--src-bypass) o_src_bypass="$2"; shift 2;;
			--src-forward) o_src_forward="$2"; shift 2;;
			--src-checkdst) o_src_checkdst="$2"; shift 2;;
			--dst-bypass) o_dst_bypass="$2"; shift 2;;
			--dst-forward) o_dst_forward="$2"; shift 2;;
			--dst-forward-recentrst) o_dst_forward_recentrst=1; shift 1;;
			--dst-bypass-file) o_dst_bypass_file="$2"; shift 2;;
			--dst-forward-file) o_dst_forward_file="$2"; shift 2;;
			*) __errmsg "unknown option $1"; return 1;;
		esac
	done

	if [ -z "$o_redir_tcp_port" -a -z "$o_redir_udp_port" ]; then
		__errmsg "Requires at least -l or -L option"
		return 1
	fi
	if [ -n "$o_dst_forward_recentrst" ] && ! iptables -m recent -h >/dev/null; then
		__errmsg "Please install iptables-mod-conntrack-extra"
		return 1
	fi
	o_remote_servers="$(for s in $o_remote_servers; do resolveip "$s" | grep_af; done)"
}

ss_rules_flush() {
	local setname

	iptables-save --counters | grep -v ss_rules_ | iptables-restore --counters
	while ip rule del fwmark 1 lookup 100 2>/dev/null; do true; done
	ip route flush table 100 2>/dev/null || true
	for setname in $(ipset -n list | grep "ss_rules${o_af}_"); do
		ipset destroy "$setname" 2>/dev/null || true
	done
}

ss_rules_ipset_init() {
	ipset --exist restore <<-EOF
		create ss_rules${o_af}_src_bypass hash:net family inet$o_af hashsize 64
		create ss_rules${o_af}_src_forward hash:net family inet$o_af hashsize 64
		create ss_rules${o_af}_src_checkdst hash:net family inet$o_af hashsize 64
		create ss_rules${o_af}_dst_bypass hash:net family inet$o_af hashsize 64
		create ss_rules${o_af}_dst_bypass_ hash:net family inet$o_af hashsize 64
		create ss_rules${o_af}_dst_forward hash:net family inet$o_af hashsize 64
		create ss_rules${o_af}_dst_forward_rrst_ hash:ip family inet$o_af hashsize 8 timeout 3600
		$(ss_rules_ipset_mkadd ss_rules${o_af}_dst_bypass_ "$o_dst_bypass_ $o_remote_servers")
		$(ss_rules_ipset_mkadd ss_rules${o_af}_src_bypass "$o_src_bypass")
		$(ss_rules_ipset_mkadd ss_rules${o_af}_src_forward "$o_src_forward")
		$(ss_rules_ipset_mkadd ss_rules${o_af}_src_checkdst "$o_src_checkdst")
		$(ss_rules_ipset_mkadd ss_rules${o_af}_dst_bypass "$o_dst_bypass $(cat "$o_dst_bypass_file" 2>/dev/null)")
		$(ss_rules_ipset_mkadd ss_rules${o_af}_dst_forward "$o_dst_forward $(cat "$o_dst_forward_file" 2>/dev/null)")
	EOF
}

ss_rules_ipset_mkadd() {
	local setname="$1"; shift
	local i

	for i in $*; do
		echo "add $setname $i"
	done | grep_af
}

ss_rules_iptchains_init() {
	ss_rules_iptchains_init_tcp
	ss_rules_iptchains_init_udp
}

ss_rules_iptchains_init_tcp() {
	local local_target

	[ -n "$o_redir_tcp_port" ] || return 0

	ss_rules_iptchains_init_ nat tcp

	case "$o_local_default" in
		checkdst) local_target=ss_rules_dst ;;
		forward) local_target=ss_rules_forward ;;
		bypass|*) return 0;;
	esac

	iptables-restore --noflush <<-EOF
		*nat
		:ss_rules_local_out -
		-I OUTPUT 1 -p tcp -j ss_rules_local_out
		-A ss_rules_local_out -m set --match-set ss_rules${o_af}_dst_bypass_ dst -j RETURN
		-A ss_rules_local_out $o_ipt_extra -j $local_target
		COMMIT
	EOF
}

ss_rules_iptchains_init_udp() {
	[ -n "$o_redir_udp_port" ] || return 0
	ss_rules_iptchains_init_ mangle udp
}

ss_rules_iptchains_init_() {
	local table="$1"
	local proto="$2"
	local forward_rules
	local src_default_target dst_default_target
	local recentrst_mangle_rules recentrst_addset_rules

	case "$proto" in
		tcp)
			forward_rules="-A ss_rules_forward -p tcp -j REDIRECT --to-ports $o_redir_tcp_port"
			if [ -n "$o_dst_forward_recentrst" ]; then
				recentrst_mangle_rules="
					*mangle
					-I PREROUTING 1 -p tcp -m tcp --tcp-flags RST RST -m recent --name ss_rules_recentrst --set --rsource
					COMMIT
				"
				recentrst_addset_rules="
					-A ss_rules_dst -m recent --name ss_rules_recentrst --rcheck --rdest --seconds 3 --hitcount 3 -j SET --add-set ss_rules${o_af}_dst_forward_rrst_ dst --exist
					-A ss_rules_dst -m set --match-set ss_rules${o_af}_dst_forward_rrst_ dst -j ss_rules_forward
				"
			fi
			;;
		udp)
			ip rule add fwmark 1 lookup 100
			ip route add local default dev lo table 100
			forward_rules="-A ss_rules_forward -p udp -j TPROXY --on-port "$o_redir_udp_port" --tproxy-mark 0x01/0x01"
			;;
	esac
	case "$o_src_default" in
		forward) src_default_target=ss_rules_forward ;;
		checkdst) src_default_target=ss_rules_dst ;;
		bypass|*) src_default_target=RETURN ;;
	esac
	case "$o_dst_default" in
		forward) dst_default_target=ss_rules_forward ;;
		bypass|*) dst_default_target=RETURN ;;
	esac
	sed -e '/^\s*$/d' -e 's/^\s\+//' <<-EOF | iptables-restore --noflush
		*$table
		:ss_rules_pre_src -
		:ss_rules_src -
		:ss_rules_dst -
		:ss_rules_forward -
		$(ss_rules_iptchains_mkprerules "$proto")
		-A ss_rules_pre_src -m set --match-set ss_rules${o_af}_dst_bypass_ dst -j RETURN
		-A ss_rules_pre_src $o_ipt_extra -j ss_rules_src
		-A ss_rules_src -m set --match-set ss_rules${o_af}_src_bypass src -j RETURN
		-A ss_rules_src -m set --match-set ss_rules${o_af}_src_forward src -j ss_rules_forward
		-A ss_rules_src -m set --match-set ss_rules${o_af}_src_checkdst src -j ss_rules_dst
		-A ss_rules_src -j $src_default_target
		-A ss_rules_dst -m set --match-set ss_rules${o_af}_dst_bypass dst -j RETURN
		-A ss_rules_dst -m set --match-set ss_rules${o_af}_dst_forward dst -j ss_rules_forward
		$recentrst_addset_rules
		-A ss_rules_dst -j $dst_default_target
		$forward_rules
		COMMIT
		$recentrst_mangle_rules
	EOF
}

ss_rules_iptchains_mkprerules() {
	local proto="$1"

	if [ -z "$o_ifnames" ]; then
		echo "-I PREROUTING 1 -p $proto -j ss_rules_pre_src"
	else
		echo $o_ifnames \
			| tr ' ' '\n' \
			| sed "s/.*/-I PREROUTING 1 -i \\0 -p $proto -j ss_rules_pre_src/"
	fi
}

ss_rules_parse_args "$@"
ss_rules_flush
ss_rules_ipset_init
ss_rules_iptchains_init
