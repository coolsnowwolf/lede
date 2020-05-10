#!/bin/sh

IPSET_LANIPLIST="laniplist"
IPSET_VPSIPLIST="vpsiplist"
IPSET_GFW="gfwlist"
IPSET_CHN="chnroute"
IPSET_BLACKLIST="blacklist"
IPSET_WHITELIST="whitelist"

ipt_n="iptables -t nat"
ipt_m="iptables -t mangle"
ip6t_n="ip6tables -t nat"
FWI=$(uci -q get firewall.passwall.path 2>/dev/null)

factor() {
	if [ -z "$1" ] || [ -z "$2" ]; then
		echo ""
	elif [ "$1" == "1:65535" ]; then
		echo ""
	else
		echo "$2 $1"
	fi
}

dst() {
	echo "-m set $2 --match-set $1 dst"
}

comment() {
	echo "-m comment --comment '$1'"
}

REDIRECT() {
	local redirect="-j REDIRECT --to-ports $1"
	[ "$2" == "TPROXY" ] && redirect="-j TPROXY --tproxy-mark 0x1/0x1 --on-port $1"
	[ "$2" == "MARK" ] && redirect="-j MARK --set-mark $1"
	echo $redirect
}

get_redirect_ipt() {
	case "$1" in
	disable)
		echo "-j RETURN"
		;;
	global)
		echo "$(REDIRECT $2 $3)"
		;;
	gfwlist)
		echo "$(dst $IPSET_GFW) $(REDIRECT $2 $3)"
		;;
	chnroute)
		echo "$(dst $IPSET_CHN !) $(REDIRECT $2 $3)"
		;;
	returnhome)
		echo "$(dst $IPSET_CHN) $(REDIRECT $2 $3)"
		;;
	esac
}

get_action_chain_name() {
	case "$1" in
	disable)
		echo "不代理"
		;;
	global)
		echo "全局代理"
		;;
	gfwlist)
		echo "GFW名单"
		;;
	chnroute)
		echo "大陆白名单"
		;;
	returnhome)
		echo "回国模式"
		;;
	esac
}

gen_laniplist() {
	cat <<-EOF
		0.0.0.0/8
		10.0.0.0/8
		100.64.0.0/10
		127.0.0.0/8
		169.254.0.0/16
		172.16.0.0/12
		192.168.0.0/16
		224.0.0.0/4
		240.0.0.0/4
	EOF
}

load_acl() {
	local count=$(uci show $CONFIG | grep "@acl_rule" | sed -n '$p' | cut -d '[' -f 2 | cut -d ']' -f 1)
	[ -n "$count" ] && [ "$count" -ge 0 ] && {
		u_get() {
			local ret=$(uci -q get $CONFIG.@acl_rule[$1].$2)
			echo ${ret:=$3}
		}
		for i in $(seq 0 $count); do
			local enabled=$(u_get $i enabled 0)
			[ "$enabled" == "0" ] && continue
			local remarks=$(u_get $i remarks)
			local ip=$(u_get $i ip)
			local mac=$(u_get $i mac)
			[ -z "$ip" -a -z "$mac" ] && continue
			local tcp_proxy_mode=$(u_get $i tcp_proxy_mode default)
			local udp_proxy_mode=$(u_get $i udp_proxy_mode default)
			local tcp_node=$(u_get $i tcp_node 1)
			local udp_node=$(u_get $i udp_node 1)
			local tcp_no_redir_ports=$(u_get $i tcp_no_redir_ports default)
			local udp_no_redir_ports=$(u_get $i udp_no_redir_ports default)
			local tcp_redir_ports=$(u_get $i tcp_redir_ports default)
			local udp_redir_ports=$(u_get $i udp_redir_ports default)
			[ "$tcp_proxy_mode" = "default" ] && tcp_proxy_mode=$TCP_PROXY_MODE
			[ "$udp_proxy_mode" = "default" ] && udp_proxy_mode=$UDP_PROXY_MODE
			[ "$TCP_NODE_NUM" == "1" ] && tcp_node=1
			[ "$UDP_NODE_NUM" == "1" ] && udp_node=1
			[ "$tcp_no_redir_ports" = "default" ] && tcp_no_redir_ports=$TCP_NO_REDIR_PORTS
			[ "$udp_no_redir_ports" = "default" ] && udp_no_redir_ports=$UDP_NO_REDIR_PORTS
			[ "$tcp_redir_ports" = "default" ] && tcp_redir_ports=$TCP_REDIR_PORTS
			[ "$udp_redir_ports" = "default" ] && udp_redir_ports=$UDP_REDIR_PORTS
			eval TCP_NODE=\$TCP_NODE$tcp_node
			eval UDP_NODE=\$UDP_NODE$udp_node
			
			if [ -n "$ip" -a -n "$mac" ]; then
				echolog "访问控制：IP：$ip，MAC：$mac，使用TCP_${tcp_node}节点，UDP_${udp_node}节点，TCP模式：$(get_action_chain_name $tcp_proxy_mode)，UDP模式：$(get_action_chain_name $udp_proxy_mode)"
			else
				[ -n "$ip" ] && echolog "访问控制：IP：$ip，使用TCP_${tcp_node}节点，UDP_${udp_node}节点，TCP模式：$(get_action_chain_name $tcp_proxy_mode)，UDP模式：$(get_action_chain_name $udp_proxy_mode)"
				[ -n "$mac" ] && echolog "访问控制：MAC：$mac，使用TCP_${tcp_node}节点，UDP_${udp_node}节点，TCP模式：$(get_action_chain_name $tcp_proxy_mode)，UDP模式：$(get_action_chain_name $udp_proxy_mode)"
			fi
			
			local ipt_tmp=$ipt_n
			[ "$tcp_proxy_mode" != "disable" ] && {
				[ "$TCP_NODE" != "nil" ] && {
					eval TCP_NODE_TYPE=$(echo $(config_n_get $TCP_NODE type) | tr 'A-Z' 'a-z')
					local is_tproxy
					[ "$TCP_NODE_TYPE" == "brook" -a "$(config_n_get $TCP_NODE brook_protocol client)" == "client" ] && ipt_tmp=$ipt_m && is_tproxy="TPROXY"
					[ "$tcp_no_redir_ports" != "disable" ] && $ipt_tmp -A PSW_ACL $(comment "$remarks") $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -p tcp -m multiport --dport $tcp_no_redir_ports -j RETURN
					eval tcp_port=\$TCP_REDIR_PORT$tcp_node
					$ipt_tmp -A PSW_ACL $(comment "$remarks") -p tcp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") $(factor $tcp_redir_ports "-m multiport --dport") $(dst $IPSET_BLACKLIST) $(REDIRECT $tcp_port $is_tproxy)
					$ipt_tmp -A PSW_ACL $(comment "$remarks") -p tcp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") $(factor $tcp_redir_ports "-m multiport --dport") $(get_redirect_ipt $tcp_proxy_mode $tcp_port $is_tproxy)
					unset is_tproxy
					unset tcp_port
				}
			}
			$ipt_tmp -A PSW_ACL $(comment "$remarks") $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -p tcp -j RETURN
			
			[ "$udp_proxy_mode" != "disable" ] && {
				[ "$UDP_NODE" != "nil" ] && {
					eval udp_port=\$UDP_REDIR_PORT$udp_node
					[ "$udp_no_redir_ports" != "disable" ] && $ipt_m -A PSW_ACL $(comment "$remarks") $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -p udp -m multiport --dport $udp_no_redir_ports -j RETURN
					$ipt_m -A PSW_ACL $(comment "$remarks") -p udp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_BLACKLIST) $(REDIRECT $udp_port TPROXY)
					$ipt_m -A PSW_ACL $(comment "$remarks") -p udp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(get_redirect_ipt $udp_proxy_mode $udp_port TPROXY)
					unset udp_port
				}
			}
			$ipt_m -A PSW_ACL $(comment "$remarks") $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -p udp -j RETURN
		done
	}
	
	#  加载TCP默认代理模式
	local ipt_tmp=$ipt_n
	[ "$TCP_NODE1" != "nil" -a "$TCP_PROXY_MODE" != "disable" ] && {
		local TCP_NODE1_TYPE=$(echo $(config_n_get $TCP_NODE1 type) | tr 'A-Z' 'a-z')
		local is_tproxy
		[ "$TCP_NODE1_TYPE" == "brook" -a "$(config_n_get $TCP_NODE1 brook_protocol client)" == "client" ] && ipt_tmp=$ipt_m && is_tproxy="TPROXY"
		[ "$TCP_NO_REDIR_PORTS" != "disable" ] && $ipt_tmp -A PSW_ACL $(comment "默认") -p tcp -m multiport --dport $TCP_NO_REDIR_PORTS -j RETURN
		$ipt_tmp -A PSW_ACL $(comment "默认") -p tcp $(factor $TCP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_BLACKLIST) $(REDIRECT $TCP_REDIR_PORT1 $is_tproxy)
		$ipt_tmp -A PSW_ACL $(comment "默认") -p tcp $(factor $TCP_REDIR_PORTS "-m multiport --dport") $(get_redirect_ipt $TCP_PROXY_MODE $TCP_REDIR_PORT1 $is_tproxy)
	}
	$ipt_tmp -A PSW_ACL $(comment "默认") -p tcp -j RETURN
	echolog "TCP默认代理模式：$(get_action_chain_name $TCP_PROXY_MODE)"
	
	#  加载UDP默认代理模式
	[ "$UDP_NODE1" != "nil" -a "$UDP_PROXY_MODE" != "disable" ] && {
		[ "$UDP_NO_REDIR_PORTS" != "disable" ] && $ipt_m -A PSW_ACL $(comment "默认") -p udp -m multiport --dport $UDP_NO_REDIR_PORTS -j RETURN
		$ipt_m -A PSW_ACL $(comment "默认") -p udp $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_BLACKLIST) $(REDIRECT $UDP_REDIR_PORT1 TPROXY)
		$ipt_m -A PSW_ACL $(comment "默认") -p udp $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(get_redirect_ipt $UDP_PROXY_MODE $UDP_REDIR_PORT1 TPROXY)
	}
	$ipt_m -A PSW_ACL $(comment "默认") -p udp -j RETURN
	echolog "UDP默认代理模式：$(get_action_chain_name $UDP_PROXY_MODE)"
}

filter_vpsip() {
	echolog "开始过滤所有IPV4节点到白名单"
	uci show $CONFIG | grep "@nodes" | grep "address" | cut -d "'" -f 2 | grep -E "([0-9]{1,3}[\.]){3}[0-9]{1,3}" | sed -e "/^$/d" | sed -e "s/^/add $IPSET_VPSIPLIST &/g" | awk '{print $0} END{print "COMMIT"}' | ipset -! -R
	echolog "过滤所有IPV4节点完成"
}

filter_node() {
	filter_rules() {
		[ -n "$1" ] && [ "$1" != "nil" ] && {
			local type=$(echo $(config_n_get $1 type) | tr 'A-Z' 'a-z')
			local i=$ipt_n
			[ "$2" == "udp" ] || [ "$type" == "brook" -a "$(config_n_get $1 brook_protocol client)" == "client" ] && i=$ipt_m
			local address=$(config_n_get $1 address)
			local port=$(config_n_get $1 port)
			
			if [ -n "$3" ] && [ "$3" == "1" ] && [ -n "$4" ]; then
				if [ "$i" == "$ipt_m" ]; then
					$i -I PSW_OUTPUT 2 -p $2 -d $address --dport $port $(REDIRECT 1 MARK)
				else
					$i -I PSW_OUTPUT 2 -p $2 -d $address --dport $port $(REDIRECT $4)
				fi
			else
				is_exist=$($i -n -L PSW_OUTPUT 2>/dev/null | grep -c "$address:$port")
				[ "$is_exist" == 0 ] && {
					local ADD_INDEX=2
					local INDEX=$($i -n -L PSW_OUTPUT --line-numbers | grep "$IPSET_VPSIPLIST" | sed -n '$p' | awk '{print $1}')
					[ -n "$INDEX" ] && ADD_INDEX=$INDEX
					$i -I PSW_OUTPUT $ADD_INDEX -p $2 -d $address --dport $port $(comment "$address:$port") -j RETURN
				}
			fi
		}
	}
	local tmp_type=$(echo $(config_n_get $1 type) | tr 'A-Z' 'a-z')
	if [ "$tmp_type" == "v2ray_shunt" ]; then
		local default_node=$(config_n_get $1 default_node nil)
		filter_rules $default_node $2
		
		local youtube_node=$(config_n_get $1 youtube_node)
		local youtube_proxy=$(config_n_get $1 youtube_proxy 0)
		[ "$default_node" == "$youtube_node" ] && youtube_proxy=0
		local netflix_node=$(config_n_get $1 netflix_node)
		local netflix_proxy=$(config_n_get $1 netflix_proxy 0)
		[ "$default_node" == "$netflix_node" ] && netflix_proxy=0
		filter_rules $(config_n_get $1 youtube_node) $2 $youtube_proxy $3
		filter_rules $(config_n_get $1 netflix_node) $2 $netflix_proxy $3
		
	elif [ "$tmp_type" == "v2ray_balancing" ]; then
		local balancing_node=$(config_n_get $1 v2ray_balancing_node)
		for node_id in $balancing_node
		do
			filter_rules $node_id $2
		done
	else
		filter_rules $1 $2
	fi
}

dns_hijack() {
	$ipt_n -I PSW -p udp --dport 53 -j REDIRECT --to-ports 53
}

add_firewall_rule() {
	echolog "开始加载防火墙规则..."
	ipset -! create $IPSET_LANIPLIST nethash
	ipset -! create $IPSET_VPSIPLIST nethash
	ipset -! create $IPSET_GFW nethash
	ipset -! create $IPSET_CHN nethash
	ipset -! create $IPSET_BLACKLIST nethash
	ipset -! create $IPSET_WHITELIST nethash

	cat $RULES_PATH/chnroute | sed -e "/^$/d" | sed -e "s/^/add $IPSET_CHN &/g" | awk '{print $0} END{print "COMMIT"}' | ipset -! -R
	cat $RULES_PATH/blacklist_ip | sed -e "/^$/d" | sed -e "s/^/add $IPSET_BLACKLIST &/g" | awk '{print $0} END{print "COMMIT"}' | ipset -! -R
	cat $RULES_PATH/whitelist_ip | sed -e "/^$/d" | sed -e "s/^/add $IPSET_WHITELIST &/g" | awk '{print $0} END{print "COMMIT"}' | ipset -! -R

	ipset -! -R <<-EOF || return 1
		$(gen_laniplist | sed -e "s/^/add $IPSET_LANIPLIST /")
	EOF
	
	# 忽略特殊IP段
	lan_ifname=$(uci -q -p /var/state get network.lan.ifname)
	[ -n "$lan_ifname" ] && {
		lan_ip=$(ip address show $lan_ifname | grep -w "inet" | awk '{print $2}')
		[ -n "$lan_ip" ] && ipset -! add $IPSET_LANIPLIST $lan_ip >/dev/null 2>&1 &
	}

	ISP_DNS=$(cat $RESOLVFILE 2>/dev/null | grep -E -o "[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+" | sort -u | grep -v 0.0.0.0 | grep -v 127.0.0.1)
	[ -n "$ISP_DNS" ] && {
		for ispip in $ISP_DNS; do
			ipset -! add $IPSET_WHITELIST $ispip >/dev/null 2>&1 &
		done
	}
	
	#  过滤所有节点IP
	filter_vpsip
	
	$ipt_n -N PSW
	$ipt_n -A PSW $(dst $IPSET_LANIPLIST) -j RETURN
	$ipt_n -A PSW $(dst $IPSET_VPSIPLIST) -j RETURN
	$ipt_n -A PSW $(dst $IPSET_WHITELIST) -j RETURN
	$ipt_n -N PSW_ACL
	
	$ipt_n -N PSW_OUTPUT
	$ipt_n -A PSW_OUTPUT $(dst $IPSET_LANIPLIST) -j RETURN
	$ipt_n -A PSW_OUTPUT $(dst $IPSET_VPSIPLIST) -j RETURN
	$ipt_n -A PSW_OUTPUT $(dst $IPSET_WHITELIST) -j RETURN

	$ipt_m -N PSW
	$ipt_m -A PSW $(dst $IPSET_LANIPLIST) -j RETURN
	$ipt_m -A PSW $(dst $IPSET_VPSIPLIST) -j RETURN
	$ipt_m -A PSW $(dst $IPSET_WHITELIST) -j RETURN
	$ipt_m -N PSW_ACL
	
	$ipt_m -N PSW_OUTPUT
	$ipt_m -A PSW_OUTPUT $(dst $IPSET_LANIPLIST) -j RETURN
	$ipt_m -A PSW_OUTPUT $(dst $IPSET_VPSIPLIST) -j RETURN
	$ipt_m -A PSW_OUTPUT $(dst $IPSET_WHITELIST) -j RETURN

	ip rule add fwmark 1 lookup 100
	ip route add local 0.0.0.0/0 dev lo table 100

	for i in $(seq 1 $SOCKS_NODE_NUM); do
		eval node=\$SOCKS_NODE$i
		[ "$node" != "nil" ] && {
			filter_node $node tcp
			filter_node $node udp
		}
	done
	
	for i in $(seq 1 $TCP_NODE_NUM); do
		eval node=\$TCP_NODE$i
		eval port=\$TCP_REDIR_PORT$i
		[ "$node" != "nil" ] && filter_node $node tcp $port
	done
	
	# 加载路由器自身代理 TCP
	if [ "$TCP_NODE1" != "nil" ]; then
		TCP_NODE1_TYPE=$(echo $(config_n_get $TCP_NODE1 type) | tr 'A-Z' 'a-z')
		if [ "$TCP_NODE1_TYPE" == "brook" -a "$(config_n_get $TCP_NODE1 brook_protocol client)" == "client" ]; then
			[ "$use_tcp_node_resolve_dns" == 1 -a -n "$DNS_FORWARD" ] && {
				for dns in $DNS_FORWARD ; do
					local dns_ip=$(echo $dns | sed "s/:/#/g" | awk -F "#" '{print $1}')
					ipset test $IPSET_LANIPLIST $dns_ip 2>/dev/null
					[ $? == 0 ] && continue
					local dns_port=$(echo $dns | sed "s/:/#/g" | awk -F "#" '{print $2}')
					[ -z "$dns_port" ] && dns_port=53
					$ipt_m -I PSW 2 -p tcp -d $dns_ip --dport $dns_port $(REDIRECT $TCP_REDIR_PORT1 TPROXY)
				done
			}
			$ipt_m -A OUTPUT -p tcp -j PSW_OUTPUT
			[ "$TCP_NO_REDIR_PORTS" != "disable" ] && $ipt_m -A PSW_OUTPUT -p tcp -m multiport --dport $TCP_NO_REDIR_PORTS -j RETURN
			$ipt_m -A PSW_OUTPUT -p tcp $(factor $TCP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_BLACKLIST) $(REDIRECT 1 MARK)
			$ipt_m -A PSW_OUTPUT -p tcp $(factor $TCP_REDIR_PORTS "-m multiport --dport") $(get_redirect_ipt $LOCALHOST_TCP_PROXY_MODE 1 MARK)
		else
			[ "$use_tcp_node_resolve_dns" == 1 -a -n "$DNS_FORWARD" ] && {
				for dns in $DNS_FORWARD ; do
					local dns_ip=$(echo $dns | sed "s/:/#/g" | awk -F "#" '{print $1}')
					ipset test $IPSET_LANIPLIST $dns_ip 2>/dev/null
					[ $? == 0 ] && continue
					local dns_port=$(echo $dns | sed "s/:/#/g" | awk -F "#" '{print $2}')
					[ -z "$dns_port" ] && dns_port=53
					local ADD_INDEX=2
					$ipt_n -I PSW_OUTPUT $ADD_INDEX -p tcp -d $dns_ip --dport $dns_port $(REDIRECT $TCP_REDIR_PORT1)
				done
			}
			$ipt_n -A OUTPUT -p tcp -j PSW_OUTPUT
			[ "$TCP_NO_REDIR_PORTS" != "disable" ] && $ipt_n -A PSW_OUTPUT -p tcp -m multiport --dport $TCP_NO_REDIR_PORTS -j RETURN
			$ipt_n -A PSW_OUTPUT -p tcp $(factor $TCP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_BLACKLIST) $(REDIRECT $TCP_REDIR_PORT1)
			$ipt_n -A PSW_OUTPUT -p tcp $(factor $TCP_REDIR_PORTS "-m multiport --dport") $(get_redirect_ipt $LOCALHOST_TCP_PROXY_MODE $TCP_REDIR_PORT1)
		fi
	fi
	
	$ipt_n -A PSW -j PSW_ACL
	local PRE_INDEX=1
	ADBYBY_INDEX=$($ipt_n -L PREROUTING --line-numbers | grep "ADBYBY" | sed -n '$p' | awk '{print $1}')
	if [ -n "$ADBYBY_INDEX" ]; then
		PRE_INDEX=$(expr $ADBYBY_INDEX + 1)
	else
		PR_INDEX=$($ipt_n -L PREROUTING --line-numbers | grep "prerouting_rule" | sed -n '$p' | awk '{print $1}')
		[ -n "$PR_INDEX" ] && PRE_INDEX=$(expr $PR_INDEX + 1)
	fi
	$ipt_n -I PREROUTING $PRE_INDEX -p tcp -j PSW
	
	if [ "$PROXY_IPV6" == "1" ]; then
		[ -n "$lan_ifname" ] && {
			lan_ipv6=$(ip address show $lan_ifname | grep -w "inet6" | awk '{print $2}') #当前LAN IPv6段
			[ -n "$lan_ipv6" ] && {
				$ip6t_n -N PSW
				$ip6t_n -N PSW_ACL
				$ip6t_n -A PREROUTING -j PSW
				[ -n "$lan_ipv6" ] && {
					for ip in $lan_ipv6; do
						$ip6t_n -A PSW -d $ip -j RETURN
					done
				}
				[ "$use_ipv6" == "1" -a -n "$server_ip" ] && $ip6t_n -A PSW -d $server_ip -j RETURN
				$ip6t_n -A PSW -p tcp $(REDIRECT $TCP_REDIR_PORT1)
				#$ip6t_n -I OUTPUT -p tcp -j PSW
			}
		}
	fi
	
	for i in $(seq 1 $UDP_NODE_NUM); do
		eval node=\$UDP_NODE$i
		eval port=\$UDP_REDIR_PORT$i
		[ "$node" != "nil" ] && filter_node $node udp $port
	done
	
	# 加载路由器自身代理 UDP
	if [ "$UDP_NODE1" != "nil" ]; then
		local UDP_NODE1_TYPE=$(echo $(config_n_get $UDP_NODE1 type) | tr 'A-Z' 'a-z')
		[ "$use_udp_node_resolve_dns" == 1 -a -n "$DNS_FORWARD" ] && {
			for dns in $DNS_FORWARD ; do
				local dns_ip=$(echo $dns | sed "s/:/#/g" | awk -F "#" '{print $1}')
				ipset test $IPSET_LANIPLIST $dns_ip 2>/dev/null
				[ $? == 0 ] && continue
				local dns_port=$(echo $dns | sed "s/:/#/g" | awk -F "#" '{print $2}')
				[ -z "$dns_port" ] && dns_port=53
				local ADD_INDEX=2
				$ipt_m -I PSW $ADD_INDEX -p udp -d $dns_ip --dport $dns_port $(REDIRECT $UDP_REDIR_PORT1 TPROXY)
				$ipt_m -I PSW_OUTPUT $ADD_INDEX -p udp -d $dns_ip --dport $dns_port $(REDIRECT 1 MARK)
			done
		}
		$ipt_m -A OUTPUT -p udp -j PSW_OUTPUT
		[ "$UDP_NO_REDIR_PORTS" != "disable" ] && $ipt_m -A PSW_OUTPUT -p udp -m multiport --dport $UDP_NO_REDIR_PORTS -j RETURN
		$ipt_m -A PSW_OUTPUT -p udp $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_BLACKLIST) $(REDIRECT 1 MARK)
		$ipt_m -A PSW_OUTPUT -p udp $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(get_redirect_ipt $LOCALHOST_UDP_PROXY_MODE 1 MARK)
	fi
	
	$ipt_m -A PSW -j PSW_ACL
	$ipt_m -A PREROUTING -j PSW
	
	#  加载ACLS
	load_acl
	
	# dns_hijack "force"
	
	echolog "防火墙规则加载完成！"
}

del_firewall_rule() {
	$ipt_n -D PREROUTING -p tcp -j PSW 2>/dev/null
	$ipt_n -D OUTPUT -p tcp -j PSW_OUTPUT 2>/dev/null
	$ipt_n -F PSW 2>/dev/null && $ipt_n -X PSW 2>/dev/null
	$ipt_n -F PSW_ACL 2>/dev/null && $ipt_n -X PSW_ACL 2>/dev/null
	$ipt_n -F PSW_OUTPUT 2>/dev/null && $ipt_n -X PSW_OUTPUT 2>/dev/null
	
	$ipt_m -D PREROUTING -j PSW 2>/dev/null
	$ipt_m -D OUTPUT -p tcp -j PSW_OUTPUT 2>/dev/null
	$ipt_m -D OUTPUT -p udp -j PSW_OUTPUT 2>/dev/null
	$ipt_m -F PSW 2>/dev/null && $ipt_m -X PSW 2>/dev/null
	$ipt_m -F PSW_ACL 2>/dev/null && $ipt_m -X PSW_ACL 2>/dev/null
	$ipt_m -F PSW_OUTPUT 2>/dev/null && $ipt_m -X PSW_OUTPUT 2>/dev/null

	$ip6t_n -D PREROUTING -j PSW 2>/dev/null
	$ip6t_n -D OUTPUT -j PSW_OUTPUT 2>/dev/null
	$ip6t_n -F PSW 2>/dev/null && $ip6t_n -X PSW 2>/dev/null
	$ip6t_n -F PSW_ACL 2>/dev/null && $ip6t_n -X PSW_ACL 2>/dev/null
	$ip6t_n -F PSW_OUTPUT 2>/dev/null && $ip6t_n -X PSW_OUTPUT 2>/dev/null
	
	ip rule del fwmark 1 lookup 100 2>/dev/null
	ip route del local 0.0.0.0/0 dev lo table 100 2>/dev/null

	ipset -F $IPSET_LANIPLIST >/dev/null 2>&1 && ipset -X $IPSET_LANIPLIST >/dev/null 2>&1 &
	ipset -F $IPSET_VPSIPLIST >/dev/null 2>&1 && ipset -X $IPSET_VPSIPLIST >/dev/null 2>&1 &
	#ipset -F $IPSET_GFW >/dev/null 2>&1 && ipset -X $IPSET_GFW >/dev/null 2>&1 &
	#ipset -F $IPSET_CHN >/dev/null 2>&1 && ipset -X $IPSET_CHN >/dev/null 2>&1 &
	#ipset -F $IPSET_BLACKLIST >/dev/null 2>&1 && ipset -X $IPSET_BLACKLIST >/dev/null 2>&1 &
	ipset -F $IPSET_WHITELIST >/dev/null 2>&1 && ipset -X $IPSET_WHITELIST >/dev/null 2>&1 &
	#echolog "删除相关防火墙规则完成。"
}

flush_ipset() {
	ipset -F $IPSET_LANIPLIST >/dev/null 2>&1 && ipset -X $IPSET_LANIPLIST >/dev/null 2>&1 &
	ipset -F $IPSET_VPSIPLIST >/dev/null 2>&1 && ipset -X $IPSET_VPSIPLIST >/dev/null 2>&1 &
	ipset -F $IPSET_GFW >/dev/null 2>&1 && ipset -X $IPSET_GFW >/dev/null 2>&1 &
	ipset -F $IPSET_CHN >/dev/null 2>&1 && ipset -X $IPSET_CHN >/dev/null 2>&1 &
	ipset -F $IPSET_BLACKLIST >/dev/null 2>&1 && ipset -X $IPSET_BLACKLIST >/dev/null 2>&1 &
	ipset -F $IPSET_WHITELIST >/dev/null 2>&1 && ipset -X $IPSET_WHITELIST >/dev/null 2>&1 &
}

flush_include() {
	echo '#!/bin/sh' >$FWI
}

gen_include() {
	flush_include
	extract_rules() {
		echo "*$1"
		iptables-save -t $1 | grep PSW | \
		sed -e "s/^-A \(OUTPUT\|PREROUTING\)/-I \1 1/"
		echo 'COMMIT'
	}
	cat <<-EOF >>$FWI
		iptables-save -c | grep -v "PSW" | iptables-restore -c
		iptables-restore -n <<-EOT
		$(extract_rules nat)
		$(extract_rules mangle)
		EOT
	EOF
	return 0
}

start() {
	add_firewall_rule
	gen_include
}

stop() {
	del_firewall_rule
	flush_include
}

case $1 in
flush_ipset)
	flush_ipset
	;;
stop)
	stop
	;;
start)
	start
	;;
*) ;;
esac
