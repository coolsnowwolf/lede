#!/bin/sh

IPSET_LANIPLIST="laniplist"
IPSET_VPSIPLIST="vpsiplist"
IPSET_GFW="gfwlist"
IPSET_CHN="chnroute"
IPSET_BLACKLIST="blacklist"
IPSET_WHITELIST="whitelist"

FORCE_INDEX=2

ipt_n="iptables -t nat"
ipt_m="iptables -t mangle"
ip6t_n="ip6tables -t nat"
ip6t_m="ip6tables -t mangle"
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

RULE_LAST_INDEX() {
	[ $# -ge 3 ] || {
		echolog "索引列举方式不正确（iptables），终止执行！"
		exit 1
	}
	local ipt_tmp=${1}; shift
	local chain=${1}; shift
	local list=${1}; shift
	local default=${1:-0}; shift
	local _index=$($ipt_tmp -n -L $chain --line-numbers 2>/dev/null | grep "$list" | sed -n '$p' | awk '{print $1}')
	echo "${_index:-${default}}"
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
	local items=$(get_enabled_anonymous_secs "@acl_rule")
	[ -n "$items" ] && {
		local item enabled remarks ip mac tcp_proxy_mode udp_proxy_mod
		local tcp_node udp_node tcp_no_redir_ports udp_no_redir_ports tcp_redir_ports udp_redir_ports
		local TCP_NODE UDP_NODE TCP_NODE_TYPE UDP_NODE_TYPE ipt_tmp is_tproxy tcp_port udp_port msg msg2
		for item in $items; do
			unset ip mac tcp_port udp_port is_tproxy msg
			eval $(uci -q show $CONFIG.${item} | cut -d'.' -sf 3- | grep -v '^$')
			[ -z "${ip}${mac}" ] && continue
			tcp_proxy_mode=${tcp_proxy_mode:-default}
			udp_proxy_mode=${udp_proxy_mode:-default}
			tcp_node=${tcp_node:-1}
			udp_node=${udp_node:-1}
			tcp_no_redir_ports=${tcp_no_redir_ports:-default}
			udp_no_redir_ports=${udp_no_redir_ports:-default}
			tcp_redir_ports=${tcp_redir_ports:-default}
			udp_redir_ports=${udp_redir_ports:-default}
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

			echolog "访问控制：${item}..."
			[ -n "$ip" ] && msg="IP：$ip，"
			[ -n "$mac" ] && msg="${msg:+${msg}和}MAC：$mac，"
			ipt_tmp=$ipt_n
			[ "$tcp_proxy_mode" != "disable" ] && {
				[ "$TCP_NODE" != "nil" ] && {
					eval tcp_port=\$TCP_REDIR_PORT$tcp_node
					eval TCP_NODE_TYPE=$(echo $(config_n_get $TCP_NODE type) | tr 'A-Z' 'a-z')
					[ "$TCP_NODE_TYPE" == "brook" ] && [ "$(config_n_get $TCP_NODE brook_protocol client)" == "client" ] && is_tproxy=1
					[ "$TCP_NODE_TYPE" == "trojan-go" ] && is_tproxy=1
					msg2="${msg}使用TCP节点${tcp_node} [$(get_action_chain_name $tcp_proxy_mode)]"
					if [ -n "${is_tproxy}" ]; then
						msg2="${msg2}(TPROXY:${tcp_port})代理"
						ipt_tmp=$ipt_m && is_tproxy="TPROXY"
					else
						msg2="${msg2}(REDIRECT:${tcp_port})代理"
					fi
					[ "$tcp_no_redir_ports" != "disable" ] && {
						$ipt_tmp -A PSW $(comment "$remarks") $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -p tcp -m multiport --dport $tcp_no_redir_ports -j RETURN
						msg2="${msg2}[$?]除${tcp_no_redir_ports}外的"
					}
					msg2="${msg2}所有端口"
					$ipt_tmp -A PSW $(comment "$remarks") -p tcp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") $(factor $tcp_redir_ports "-m multiport --dport") $(dst $IPSET_BLACKLIST) $(REDIRECT $tcp_port $is_tproxy)
					$ipt_tmp -A PSW $(comment "$remarks") -p tcp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") $(factor $tcp_redir_ports "-m multiport --dport") $(get_redirect_ipt $tcp_proxy_mode $tcp_port $is_tproxy)
				}
				echolog "  - ${msg2}"
			}
			$ipt_tmp -A PSW $(comment "$remarks") $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -p tcp -j RETURN
			
			[ "$udp_proxy_mode" != "disable" ] && {
				msg2="${msg}使用UDP节点${udp_node} [$(get_action_chain_name $udp_proxy_mode)]"
				[ "$UDP_NODE" != "nil" ] && {
					eval udp_port=\$UDP_REDIR_PORT$udp_node
					msg2="${msg2}(TPROXY:${udp_port})代理"
					[ "$udp_no_redir_ports" != "disable" ] && {
						$ipt_m -A PSW $(comment "$remarks") $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -p udp -m multiport --dport $udp_no_redir_ports -j RETURN
						msg2="${msg2}[$?]除${udp_no_redir_ports}外的"
					}
					msg2="${msg2}所有端口"
					$ipt_m -A PSW $(comment "$remarks") -p udp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_BLACKLIST) $(REDIRECT $udp_port TPROXY)
					$ipt_m -A PSW $(comment "$remarks") -p udp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(get_redirect_ipt $udp_proxy_mode $udp_port TPROXY)
				}
				echolog "  - ${msg2}"
			}
			$ipt_m -A PSW $(comment "$remarks") $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -p udp -j RETURN
		done
	}

	#  加载TCP默认代理模式
	local ipt_tmp=$ipt_n
	local is_tproxy msg
	unset is_tproxy msg
	[ "$TCP_NODE1" != "nil" ] && [ "$TCP_PROXY_MODE" != "disable" ] && {
		local TCP_NODE1_TYPE=$(echo $(config_n_get $TCP_NODE1 type) | tr 'A-Z' 'a-z')
		[ "$TCP_NODE1_TYPE" == "brook" ] && [ "$(config_n_get $TCP_NODE1 brook_protocol client)" == "client" ] && is_tproxy=1
		[ "$TCP_NODE1_TYPE" == "trojan-go" ] && is_tproxy=1
			msg="TCP默认代理：使用TCP节点1 [$(get_action_chain_name $TCP_PROXY_MODE)]"
		if [ -n "$is_tproxy" ]; then
			ipt_tmp=$ipt_m && is_tproxy="TPROXY"
			msg="${msg}(TPROXY:${TCP_REDIR_PORT1})代理"
		else
			msg="${msg}(REDIRECT:${TCP_REDIR_PORT1})代理"
		fi
		[ "$TCP_NO_REDIR_PORTS" != "disable" ] && $ipt_tmp -A PSW $(comment "默认") -p tcp -m multiport --dport $TCP_NO_REDIR_PORTS -j RETURN && msg="除${TCP_NO_REDIR_PORTS}外的"
		msg="${msg}所有端口"
		$ipt_tmp -A PSW $(comment "默认") -p tcp $(factor $TCP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_BLACKLIST) $(REDIRECT $TCP_REDIR_PORT1 $is_tproxy)
		$ipt_tmp -A PSW $(comment "默认") -p tcp $(factor $TCP_REDIR_PORTS "-m multiport --dport") $(get_redirect_ipt $TCP_PROXY_MODE $TCP_REDIR_PORT1 $is_tproxy)
		echolog "${msg}"
	}
	$ipt_tmp -A PSW $(comment "默认") -p tcp -j RETURN

	#  加载UDP默认代理模式
	if [ "$UDP_NODE1" != "nil" ] && [ "$UDP_PROXY_MODE" != "disable" ]; then
		msg="UDP默认代理：使用UDP节点1 [$(get_action_chain_name $UDP_PROXY_MODE)](TPROXY:${UDP_REDIR_PORT1})代理"
		[ "$UDP_NO_REDIR_PORTS" != "disable" ] && $ipt_m -A PSW $(comment "默认") -p udp -m multiport --dport $UDP_NO_REDIR_PORTS -j RETURN && msg="除${TCP_NO_REDIR_PORTS}外的"
		msg="${msg}所有端口"
		$ipt_m -A PSW $(comment "默认") -p udp $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_BLACKLIST) $(REDIRECT $UDP_REDIR_PORT1 TPROXY)
		$ipt_m -A PSW $(comment "默认") -p udp $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(get_redirect_ipt $UDP_PROXY_MODE $UDP_REDIR_PORT1 TPROXY)
		echolog "${msg}"
	fi
	$ipt_m -A PSW $(comment "默认") -p udp -j RETURN
}

filter_vpsip() {
	uci show $CONFIG | grep ".address=" | cut -d "'" -f 2 | grep -E "([0-9]{1,3}[\.]){3}[0-9]{1,3}" | sed -e "/^$/d" | sed -e "s/^/add $IPSET_VPSIPLIST &/g" | awk '{print $0} END{print "COMMIT"}' | ipset -! -R
	#uci show $CONFIG | grep ".address=" | cut -d "'" -f 2 | grep -E "([[a-f0-9]{1,4}(:[a-f0-9]{1,4}){7}|[a-f0-9]{1,4}(:[a-f0-9]{1,4}){0,7}::[a-f0-9]{0,4}(:[a-f0-9]{1,4}){0,7}])" | sed -e "/^$/d" | sed -e "s/^/add $IPSET_VPSIP6LIST &/g" | awk '{print $0} END{print "COMMIT"}' | ipset -! -R
	echolog "过滤所有节点直接 IP 地址完成[$?]"
}

filter_node() {
	local proxy_node=${1}
	local stream=$(echo ${2} | tr 'A-Z' 'a-z')
	local proxy_port=${3}

	filter_rules() {
		local node=${1}
		local stream=${2}
		local _proxy=${3}
		local _port=${4}
		local is_tproxy ipt_tmp ip6t_tmp msg msg2

		if [ -n "$node" ] && [ "$node" != "nil" ]; then
			local type=$(echo $(config_n_get $node type) | tr 'A-Z' 'a-z')
			local address=$(config_n_get $node address)
			local port=$(config_n_get $node port)
			ipt_tmp=$ipt_n
			ip6t_tmp=$ip6t_n
			[ "$stream" == "udp" ] && is_tproxy=1
			[ "$type" == "brook" ] && [ "$(config_n_get $node brook_protocol client)" == "client" ] && is_tproxy=1
			[ "$type" == "trojan-go" ] && is_tproxy=1
			if [ -n "$is_tproxy" ]; then
				ipt_tmp=$ipt_m
				ip6t_tmp=$ip6t_m
				msg="TPROXY"
			else
				msg="REDIRECT"
			fi
		else
			echolog "  - 节点配置不正常，略过"
			return 0
		fi

		local ADD_INDEX=$FORCE_INDEX
		for _ipt in 4 6; do
			[ "$_ipt" == "4" ] && _ipt=$ipt_tmp
			[ "$_ipt" == "6" ] && _ipt=$ip6t_tmp
			$_ipt -n -L PSW_OUTPUT | grep -q "${address}:${port}"
			if [ $? -ne 0 ]; then
				local dst_rule=$(REDIRECT 1 MARK)
				msg2="按规则路由(${msg})"
				[ "$_ipt" == "$ipt_m" ] || {
					dst_rule=$(REDIRECT $_port)
					msg2="套娃使用(${msg}:${port}>>${_port})"
				}
				[ "$_ipt" == "$ip6t_tmp" ] || {
					dst_rule=$(REDIRECT $_port)
					msg2="套娃使用(${msg}:${port}>>${_port})"
				}
				[ -n "$_proxy" ] && [ "$_proxy" == "1" ] && [ -n "$_port" ] || {
					ADD_INDEX=$(RULE_LAST_INDEX "$_ipt" PSW_OUT_PUT "$IPSET_VPSIPLIST" $FORCE_INDEX)
					dst_rule=" -j RETURN"
					msg2="直连代理(${msg})"
				}
				$_ipt -I PSW_OUTPUT $ADD_INDEX $(comment "${address}:${port}") -p $stream -d $address --dport $port $dst_rule 2>/dev/null
			else
				msg2="已配置过的节点，"
			fi
		done
		msg="[$?]${msg2}使用链${ADD_INDEX}，节点（${type}）：${address}:${port}"
		echolog "  - ${msg}"
	}
	
	local proxy_protocol=$(config_n_get $proxy_node protocol)
	local proxy_type=$(echo $(config_n_get $proxy_node type nil) | tr 'A-Z' 'a-z')
	[ "$proxy_type" == "nil" ] && echolog "  - 节点配置不正常，略过！：${proxy_node}" && return 0
	if [ "$proxy_protocol" == "_shunt" ]; then
		echolog "  - 按请求目的地址分流（${proxy_type}）..."
		local default_node=$(config_n_get $proxy_node default_node nil)
		filter_rules $default_node $stream
		local default_node_address=$(get_host_ip ipv4 $(config_n_get $default_node address) 1)
		local default_node_port=$(config_n_get $default_node port)
		
		local shunt_ids=$(uci show $CONFIG | grep "=shunt_rules" | awk -F '.' '{print $2}' | awk -F '=' '{print $1}')
		for shunt_id in $shunt_ids; do
			local shunt_proxy=$(config_n_get $proxy_node "${shunt_id}_proxy" 0)
			local shunt_node=$(config_n_get $proxy_node "${shunt_id}" nil)
			[ "$shunt_proxy" == 1 ] && {
				local shunt_node_address=$(get_host_ip ipv4 $(config_n_get $shunt_node address) 1)
				local shunt_node_port=$(config_n_get $shunt_node port)
				[ "$shunt_node_address" == "$default_node_address" ] && [ "$shunt_node_port" == "$default_node_port" ] && {
					shunt_proxy=0
				}
			}
			filter_rules "$(config_n_get $proxy_node $shunt_id)" "$stream" "$shunt_proxy" "$proxy_port"
		done
	elif [ "$proxy_protocol" == "_balancing" ]; then
		echolog "  - 多节点负载均衡（${proxy_type}）..."
		proxy_node=$(config_n_get $proxy_node balancing_node)
		for _node in $proxy_node; do
			filter_rules "$_node" "$stream"
		done
	else
		echolog "  - 普通节点（${proxy_type}）..."
		filter_rules "$proxy_node" "$stream"
	fi
}

dns_hijack() {
	$ipt_n -I PSW -p udp --dport 53 -j REDIRECT --to-ports 53
	echolog "强制转发本机DNS端口 UDP/53 的请求[$?]"
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
	cat $RULES_PATH/proxy_ip | sed -e "/^$/d" | sed -e "s/^/add $IPSET_BLACKLIST &/g" | awk '{print $0} END{print "COMMIT"}' | ipset -! -R
	cat $RULES_PATH/direct_ip | sed -e "/^$/d" | sed -e "s/^/add $IPSET_WHITELIST &/g" | awk '{print $0} END{print "COMMIT"}' | ipset -! -R

	ipset -! -R <<-EOF
		$(gen_laniplist | sed -e "s/^/add $IPSET_LANIPLIST /")
	EOF
	[ $? -eq 0 ] || {
		echolog "系统不兼容，终止执行！"
		return 1
	}
	
	# 忽略特殊IP段
	local lan_ifname lan_ip
	lan_ifname=$(uci -q -p /var/state get network.lan.ifname)
	[ -n "$lan_ifname" ] && {
		lan_ip=$(ip address show $lan_ifname | grep -w "inet" | awk '{print $2}')
		echolog "本机网段互访直连：${lan_ip}"
		[ -n "$lan_ip" ] && ipset -! add $IPSET_LANIPLIST $lan_ip >/dev/null 2>&1 &
	}

	local ISP_DNS=$(cat $RESOLVFILE 2>/dev/null | grep -E -o "[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+" | sort -u | grep -v 0.0.0.0 | grep -v 127.0.0.1)
	[ -n "$ISP_DNS" ] && {
		echolog "处理 ISP DNS 例外..."
		for ispip in $ISP_DNS; do
			ipset -! add $IPSET_WHITELIST $ispip >/dev/null 2>&1 &
			echolog "  - 追加到白名单：${ispip}"
		done
	}
	
	#  过滤所有节点IP
	filter_vpsip
	
	$ipt_n -N PSW
	$ipt_n -A PSW $(dst $IPSET_LANIPLIST) -j RETURN
	$ipt_n -A PSW $(dst $IPSET_VPSIPLIST) -j RETURN
	$ipt_n -A PSW $(dst $IPSET_WHITELIST) -j RETURN
	
	$ipt_n -N PSW_OUTPUT
	$ipt_n -A PSW_OUTPUT $(dst $IPSET_LANIPLIST) -j RETURN
	$ipt_n -A PSW_OUTPUT $(dst $IPSET_VPSIPLIST) -j RETURN
	$ipt_n -A PSW_OUTPUT $(dst $IPSET_WHITELIST) -j RETURN

	$ipt_m -N PSW
	$ipt_m -A PSW $(dst $IPSET_LANIPLIST) -j RETURN
	$ipt_m -A PSW $(dst $IPSET_VPSIPLIST) -j RETURN
	$ipt_m -A PSW $(dst $IPSET_WHITELIST) -j RETURN
	
	$ipt_m -N PSW_OUTPUT
	$ipt_m -A PSW_OUTPUT $(dst $IPSET_LANIPLIST) -j RETURN
	$ipt_m -A PSW_OUTPUT $(dst $IPSET_VPSIPLIST) -j RETURN
	$ipt_m -A PSW_OUTPUT $(dst $IPSET_WHITELIST) -j RETURN

	ip rule add fwmark 1 lookup 100
	ip route add local 0.0.0.0/0 dev lo table 100
	
	# 加载路由器自身代理 TCP
	if [ "$TCP_NODE1" != "nil" ]; then
		local ipt_tmp=$ipt_n
		local dns_l="PSW_OUTPUT"
		local dns_r=$(REDIRECT $TCP_REDIR_PORT1)
		local blist_r=$(REDIRECT $TCP_REDIR_PORT1)
		local p_r=$(get_redirect_ipt $LOCALHOST_TCP_PROXY_MODE $TCP_REDIR_PORT1)
		TCP_NODE1_TYPE=$(echo $(config_n_get $TCP_NODE1 type) | tr 'A-Z' 'a-z')
		echolog "加载路由器自身 TCP 代理..."
		if [ "$TCP_NODE1_TYPE" == "brook" ] && [ "$(config_n_get $TCP_NODE1 brook_protocol client)" == "client" ]; then
			echolog "  - 启用 TPROXY 模式"
			ipt_tmp=$ipt_m
			dns_l="PSW"
			dns_r="$(REDIRECT $TCP_REDIR_PORT1 TPROXY)"
			blist_r=$(REDIRECT 1 MARK)
			p_r=$(get_redirect_ipt $LOCALHOST_TCP_PROXY_MODE 1 MARK)
		fi
		_proxy_tcp_access() {
			[ -n "${2}" ] || return 0
			ipset test $IPSET_LANIPLIST ${2} 2>/dev/null
			[ $? -eq 0 ] && {
				echolog "  - 上游 DNS 服务器 ${2} 已在直接访问的列表中，不强制向 TCP 代理转发对该服务器 TCP/${3} 端口的访问"
				return 0
			}
			local ADD_INDEX=$FORCE_INDEX
			$ipt_tmp -I $dns_l $ADD_INDEX -p tcp -d ${2} --dport ${3} $dns_r
			[ "$ipt_tmp" == "$ipt_m" ] && $ipt_tmp -I PSW_OUTPUT $ADD_INDEX -p tcp -d ${2} --dport ${3} $(REDIRECT 1 MARK)
			echolog "  - [$?]将上游 DNS 服务器 ${2}:${3} 加入到路由器自身代理的 TCP 转发链${ADD_INDEX}"
		}
		[ "$use_tcp_node_resolve_dns" == 1 ] && hosts_foreach DNS_FORWARD _proxy_tcp_access 53
		$ipt_tmp -A OUTPUT -p tcp -j PSW_OUTPUT
		[ "$TCP_NO_REDIR_PORTS" != "disable" ] && {
			$ipt_tmp -A PSW_OUTPUT -p tcp -m multiport --dport $TCP_NO_REDIR_PORTS -j RETURN
			echolog "  - [$?]不代理TCP 端口：$TCP_NO_REDIR_PORTS"
		}
		$ipt_tmp -A PSW_OUTPUT -p tcp $(factor $TCP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_BLACKLIST) $blist_r
		$ipt_tmp -A PSW_OUTPUT -p tcp $(factor $TCP_REDIR_PORTS "-m multiport --dport") $p_r
	fi

	local PR_INDEX=$(RULE_LAST_INDEX "$ipt_n" PREROUTING ADBYBY)
	if [ "$PR_INDEX" == "0" ]; then
		PR_INDEX=$(RULE_LAST_INDEX "$ipt_n" PREROUTING prerouting_rule)
	else
		echolog "发现 adbyby 规则链，adbyby 规则优先..."
	fi
	PR_INDEX=$((PR_INDEX + 1))
	$ipt_n -I PREROUTING $PR_INDEX -p tcp -j PSW
	echolog "使用链表 PREROUTING 排列索引${PR_INDEX}[$?]"
	
	$ip6t_n -N PSW
	$ip6t_n -A PREROUTING -j PSW
	$ip6t_n -N PSW_OUTPUT
	$ip6t_n -A OUTPUT -p tcp -j PSW_OUTPUT
	
	$ip6t_m -N PSW
	$ip6t_m -A PREROUTING -j PSW
	$ip6t_m -N PSW_OUTPUT
	$ip6t_m -A OUTPUT -p tcp -j PSW_OUTPUT
	[ -n "$lan_ifname" ] && {
		lan_ipv6=$(ip address show $lan_ifname | grep -w "inet6" | awk '{print $2}') #当前LAN IPv6段
		[ -n "$lan_ipv6" ] && {
			for ip in $lan_ipv6; do
				$ip6t_n -A PSW -d $ip -j RETURN
				$ip6t_n -A PSW_OUTPUT -d $ip -j RETURN
			done
		}
	}
	
	if [ "$PROXY_IPV6" == "1" ]; then
		local msg="IPv6 配置不当，无法代理"
		$ip6t_n -A PSW -p tcp $(REDIRECT $TCP_REDIR_PORT1)
		$ip6t_n -A PSW_OUTPUT -p tcp $(REDIRECT $TCP_REDIR_PORT1)
		msg="${msg}，转发 IPv6 TCP 流量到节点1[$?]"
		echolog "$msg"
	fi

	# 过滤Socks节点
	local ids=$(uci show $CONFIG | grep "=socks" | awk -F '.' '{print $2}' | awk -F '=' '{print $1}')
	echolog "分析 Socks 服务所使用节点..."
	local id enabled node port msg num
	for id in $ids; do
		enabled=$(config_n_get $id enabled 0)
		[ "$enabled" == "1" ] || continue
		node=$(config_n_get $id node nil)
		port=$(config_n_get $id port 0)
		msg="Socks 服务 [:${port}]"
		if [ "$node" == "nil" ] || [ "$port" == "0" ]; then
			msg="${msg} 未配置完全，略过"
		elif [ "$(echo $node | grep ^tcp)" ]; then
			num=$(echo $node | sed "s/tcp//g")
			eval "node=\${TCP_NODE$num}"
			msg="${msg} 使用与 TCP 代理自动切换${num} 相同的节点，延后处理"
		else
			filter_node $node tcp
			filter_node $node udp
		fi
		echolog "  - ${msg}"
	done

	# 处理轮换节点的分流或套娃
	local node port stream switch
	for stream in TCP UDP; do
		for switch in $(eval "seq 1 \${${stream}_NODE_NUM}"); do
			eval "node=\${${stream}_NODE$switch}"
			eval "port=\${${stream}_REDIR_PORT$switch}"
			echolog "分析 $stream 代理自动切换$switch..."
			[ "$node" == "tcp" ] && [ "$stream" == "UDP" ] && {
				eval "node=\${TCP_NODE$switch}"
				eval "port=\${TCP_REDIR_PORT$switch}"
				echolog "  - 采用 TCP 代理的配置"
			}

			if [ "$node" != "nil" ]; then
				filter_node $node $stream $port
			else
				echolog "  - 忽略无效的 $stream 代理自动切换$switch"
			fi
		done
	done
	
	# 加载路由器自身代理 UDP
	if [ "$UDP_NODE1" != "nil" ]; then
		echolog "加载路由器自身 UDP 代理..."
		local UDP_NODE1_TYPE=$(echo $(config_n_get $UDP_NODE1 type) | tr 'A-Z' 'a-z')
		local ADD_INDEX=$FORCE_INDEX
		_proxy_udp_access() {
			[ -n "${2}" ] || return 0
			ipset test $IPSET_LANIPLIST ${2} 2>/dev/null
			[ $? == 0 ] && {
				echolog "  - 上游 DNS 服务器 ${2} 已在直接访问的列表中，不强制向 UDP 代理转发对该服务器 UDP/${3} 端口的访问"
				return 0
			}
			$ipt_m -I PSW $ADD_INDEX -p udp -d ${2} --dport ${3} $(REDIRECT $UDP_REDIR_PORT1 TPROXY)
			$ipt_m -I PSW_OUTPUT $ADD_INDEX -p udp -d ${2} --dport ${3} $(REDIRECT 1 MARK)
			echolog "  - [$?]将上游 DNS 服务器 ${2}:${3} 加入到路由器自身代理的 UDP 转发链${ADD_INDEX}"
		}
		[ "$use_udp_node_resolve_dns" == 1 ] && hosts_foreach DNS_FORWARD _proxy_udp_access 53
		$ipt_m -A OUTPUT -p udp -j PSW_OUTPUT
		[ "$UDP_NO_REDIR_PORTS" != "disable" ] && {
			$ipt_m -A PSW_OUTPUT -p udp -m multiport --dport $UDP_NO_REDIR_PORTS -j RETURN
			echolog "  - [$?]不代理 UDP 端口：$UDP_NO_REDIR_PORTS"
		}
		$ipt_m -A PSW_OUTPUT -p udp $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_BLACKLIST) $(REDIRECT 1 MARK)
		$ipt_m -A PSW_OUTPUT -p udp $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(get_redirect_ipt $LOCALHOST_UDP_PROXY_MODE 1 MARK)
	fi
	
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
	$ipt_n -F PSW_OUTPUT 2>/dev/null && $ipt_n -X PSW_OUTPUT 2>/dev/null
	
	$ipt_m -D PREROUTING -j PSW 2>/dev/null
	$ipt_m -D OUTPUT -p tcp -j PSW_OUTPUT 2>/dev/null
	$ipt_m -D OUTPUT -p udp -j PSW_OUTPUT 2>/dev/null
	$ipt_m -F PSW 2>/dev/null && $ipt_m -X PSW 2>/dev/null
	$ipt_m -F PSW_OUTPUT 2>/dev/null && $ipt_m -X PSW_OUTPUT 2>/dev/null

	$ip6t_n -D PREROUTING -j PSW 2>/dev/null
	$ip6t_n -D OUTPUT -p tcp -j PSW_OUTPUT 2>/dev/null
	$ip6t_n -F PSW 2>/dev/null && $ip6t_n -X PSW 2>/dev/null
	$ip6t_n -F PSW_OUTPUT 2>/dev/null && $ip6t_n -X PSW_OUTPUT 2>/dev/null
	
	$ip6t_m -D PREROUTING -j PSW 2>/dev/null
	$ip6t_m -D OUTPUT -p tcp -j PSW_OUTPUT 2>/dev/null
	$ip6t_m -F PSW 2>/dev/null && $ip6t_m -X PSW 2>/dev/null
	$ip6t_m -F PSW_OUTPUT 2>/dev/null && $ip6t_m -X PSW_OUTPUT 2>/dev/null
	
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
		local _ipt="iptables"
		[ "$1" == "6" ] && _ipt="ip6tables"
	
		echo "*$2"
		${_ipt}-save -t $2 | grep PSW | \
		sed -e "s/^-A \(OUTPUT\|PREROUTING\)/-I \1 1/"
		echo 'COMMIT'
	}
	cat <<-EOF >>$FWI
		iptables-save -c | grep -v "PSW" | iptables-restore -c
		iptables-restore -n <<-EOT
		$(extract_rules 4 nat)
		$(extract_rules 4 mangle)
		EOT
		ip6tables-save -c | grep -v "PSW" | ip6tables-restore -c
		ip6tables-restore -n <<-EOT
		$(extract_rules 6 nat)
		$(extract_rules 6 mangle)
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
